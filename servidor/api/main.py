"""API do PlantCare
Documentação automática: http://localhost:8000/docs
"""
import asyncio
import logging
import os
from contextlib import asynccontextmanager
from datetime import datetime, timedelta, timezone
from typing import Optional

import httpx
import pymysql
from fastapi import FastAPI, Header, Request, Response
from fastapi.exceptions import RequestValidationError
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse

from bd import ligar
from meteorologia import CidadeNaoEncontrada, ciclo_meteo, geocodificar
from modelos import Leitura, PedidoPin, PedidoRegisto
from seguranca import hash_chave, hash_pin, pin_correto

logging.basicConfig(level=logging.INFO)
log = logging.getLogger("plantcare.api")

VALIDADE_PIN = timedelta(hours=24)
MAX_TENTATIVAS_PIN = 5
MENSAGEM_PIN_INVALIDO = "Código ou PIN inválidos."   # a mesma para todos os casos: não dá pistas


# ---------------------------------------------------------------------------
# Arranque e paragem da API
# ---------------------------------------------------------------------------
@asynccontextmanager
async def ciclo_de_vida(app: FastAPI):
    """O que corre quando a API liga (antes do yield) e quando desliga (depois)."""
    tarefa_meteo = None
    if os.getenv("METEO_ATIVA", "1") == "1":
        tarefa_meteo = asyncio.create_task(ciclo_meteo())
    yield
    if tarefa_meteo:
        tarefa_meteo.cancel()


app = FastAPI(
    title="PlantCare API",
    version="0.3",
    description="API do PlantCare.",
    lifespan=ciclo_de_vida,
)

# CORS: os formulários do Grafana correm no browser, noutra "origem" (porta 3000).
# Sem isto, o browser bloqueava os pedidos deles à API (porta 8000).
app.add_middleware(
    CORSMiddleware,
    allow_origins=[os.getenv("GRAFANA_URL", "http://localhost:3000")],
    allow_methods=["POST"],
    allow_headers=["Content-Type"],
)


# ---------------------------------------------------------------------------
# Erros no formato do contrato:  { "erro": "...", "mensagem": "...", "detalhes": [...] }
# ---------------------------------------------------------------------------
class ErroAPI(Exception):
    def __init__(self, estado: int, erro: str, mensagem: str = None, detalhes: list = None):
        self.estado, self.erro, self.mensagem, self.detalhes = estado, erro, mensagem, detalhes


@app.exception_handler(ErroAPI)
async def tratar_erro_api(request: Request, e: ErroAPI):
    corpo = {"erro": e.erro}
    if e.mensagem:
        corpo["mensagem"] = e.mensagem
    if e.detalhes:
        corpo["detalhes"] = e.detalhes
    return JSONResponse(status_code=e.estado, content=corpo)


@app.exception_handler(RequestValidationError)
async def tratar_dados_invalidos(request: Request, e: RequestValidationError):
    """O FastAPI responde 422 por defeito; o contrato diz 400. Aqui trocamos."""
    detalhes = []
    for problema in e.errors():
        if problema["type"] == "json_invalid":
            detalhes.append("corpo: o JSON está mal formado")
            continue
        campo = ".".join(str(parte) for parte in problema["loc"][1:]) or "corpo"
        detalhes.append(f"{campo}: {problema['msg']}")
    return JSONResponse(status_code=400, content={"erro": "dados_invalidos", "detalhes": detalhes})


# ---------------------------------------------------------------------------
# Ajudas
# ---------------------------------------------------------------------------
def agora_utc() -> datetime:
    return datetime.now(timezone.utc).replace(tzinfo=None, microsecond=0)


def para_utc(instante: datetime) -> datetime:
    """Converte para UTC "sem fuso" (como o MySQL guarda). Sem fuso indicado, assume UTC."""
    if instante.tzinfo is not None:
        instante = instante.astimezone(timezone.utc).replace(tzinfo=None)
    return instante.replace(microsecond=0)


def iso(instante: datetime) -> str:
    """2026-11-11 20:00:00  →  "2026-11-11T20:00:00Z" (formato do contrato)."""
    return instante.isoformat() + "Z"


def vaso_pela_chave(c, chave: Optional[str]) -> dict:
    """Descobre o vaso a partir do cabeçalho X-API-Key. Sem chave ou chave errada → 401."""
    if not chave:
        raise ErroAPI(401, "chave_em_falta", "Falta o cabeçalho X-API-Key.")
    c.execute(
        "SELECT id, codigo, utilizador_id FROM dispositivos WHERE chave_api_hash = %s",
        (hash_chave(chave),),
    )
    vaso = c.fetchone()
    if vaso is None:
        raise ErroAPI(401, "chave_invalida", "Chave do dispositivo inválida.")
    return vaso


# ---------------------------------------------------------------------------
# GET /api/saude
# ---------------------------------------------------------------------------
@app.get("/api/saude")
def saude():
    """O servidor está vivo? Também confirma que consegue falar com a BD."""
    try:
        with ligar() as bd, bd.cursor() as c:
            c.execute("SELECT 1")
    except Exception as erro:
        log.error("Sem ligação à BD: %s", erro)
        return JSONResponse(status_code=503, content={"estado": "sem_bd"})
    return {"estado": "ok"}


# ---------------------------------------------------------------------------
# POST /api/leituras
# ---------------------------------------------------------------------------
@app.post("/api/leituras", status_code=201)
def receber_leitura(leitura: Leitura, response: Response, x_api_key: Optional[str] = Header(default=None)):
    """O ESP32 envia uma leitura dos sensores."""
    agora = agora_utc()
    medida_em = para_utc(leitura.data_hora)

    with ligar() as bd, bd.cursor() as c:
        # 1. Que vaso é? (pela chave)
        vaso = vaso_pela_chave(c, x_api_key)

        # 2. O vaso comunicou: atualiza já, mesmo que a leitura seja recusada a seguir.
        #    O commit grava isto antes de qualquer erro desfazer o resto.
        c.execute("UPDATE dispositivos SET ultima_comunicacao = %s WHERE id = %s", (agora, vaso["id"]))
        bd.commit()

        # 3. A data da medição faz sentido? (até 7 dias no passado, no máximo 5 min no futuro)
        if medida_em < agora - timedelta(days=7):
            raise ErroAPI(400, "dados_invalidos", detalhes=["data_hora: tem mais de 7 dias"])
        if medida_em > agora + timedelta(minutes=5):
            raise ErroAPI(400, "dados_invalidos", detalhes=["data_hora: está no futuro (máximo 5 min)"])

        # 4. Qual é a planta ativa neste vaso? (a BD garante que há no máximo uma)
        c.execute("SELECT id FROM plantas WHERE dispositivo_id = %s AND ativa", (vaso["id"],))
        planta = c.fetchone()
        if planta is None:
            raise ErroAPI(409, "sem_planta_ativa", "Este vaso ainda não tem nenhuma planta registada.")

        # 5. Guarda a leitura
        try:
            c.execute(
                """INSERT INTO leituras
                   (planta_id, data_hora, recebido_em, humidade_solo, temperatura, humidade_ar, luz, nivel_agua)
                   VALUES (%s, %s, %s, %s, %s, %s, %s, %s)""",
                (planta["id"], medida_em, agora, leitura.humidade_solo, leitura.temperatura,
                 leitura.humidade_ar, leitura.luz, leitura.nivel_agua),
            )
            id_leitura = c.lastrowid
        except pymysql.err.IntegrityError as erro:
            if erro.args[0] != 1062:        # 1062 = "Duplicate entry": não é esse o problema → erro a sério
                raise
            # Reenvio depois de uma falha de rede: já existe. Responde 200 com a que já lá está.
            c.execute(
                "SELECT id, recebido_em FROM leituras WHERE planta_id = %s AND data_hora = %s",
                (planta["id"], medida_em),
            )
            existente = c.fetchone()
            response.status_code = 200
            return {"id": existente["id"], "planta_id": planta["id"], "recebido_em": iso(existente["recebido_em"])}

    return {"id": id_leitura, "planta_id": planta["id"], "recebido_em": iso(agora)}


# ---------------------------------------------------------------------------
# POST /api/dispositivos/pin
# ---------------------------------------------------------------------------
@app.post("/api/dispositivos/pin")
def guardar_pin(pedido: PedidoPin, x_api_key: Optional[str] = Header(default=None)):
    """O vaso envia o PIN que mostrou no portal e no ecrã."""
    agora = agora_utc()
    with ligar() as bd, bd.cursor() as c:
        vaso = vaso_pela_chave(c, x_api_key)
        c.execute("UPDATE dispositivos SET ultima_comunicacao = %s WHERE id = %s", (agora, vaso["id"]))

        if vaso["utilizador_id"] is not None:
            bd.commit()                     # grava a última comunicação antes de responder com erro
            raise ErroAPI(409, "vaso_ja_tem_dono", "Este vaso já está registado: o PIN não é preciso.")

        expira_em = agora + VALIDADE_PIN
        c.execute(
            "UPDATE dispositivos SET pin_hash = %s, pin_expira_em = %s, pin_tentativas = 0 WHERE id = %s",
            (hash_pin(pedido.pin), expira_em, vaso["id"]),
        )
    return {"codigo": vaso["codigo"], "pin_expira_em": iso(expira_em)}


# ---------------------------------------------------------------------------
# POST /api/plantas/registar
# ---------------------------------------------------------------------------
@app.post("/api/plantas/registar", status_code=201)
def registar_vaso(pedido: PedidoRegisto):
    """O formulário do Grafana regista o vaso e a planta na conta do utilizador."""
    agora = agora_utc()
    codigo = pedido.codigo.strip().upper()

    # 1. Cidade → coordenadas. Fica FORA da transação: é um pedido à internet que pode demorar,
    #    e não queremos a linha do vaso bloqueada enquanto esperamos.
    try:
        latitude, longitude = geocodificar(pedido.cidade)
    except CidadeNaoEncontrada:
        raise ErroAPI(400, "dados_invalidos", detalhes=[f"cidade: '{pedido.cidade}' não encontrada"])
    except httpx.HTTPError as erro:
        # O Open-Meteo está em baixo: regista na mesma, sem coordenadas.
        # A tarefa da meteorologia preenche-as na próxima recolha.
        log.warning("Geocodificação indisponível (%s): o vaso fica sem coordenadas por agora", erro)
        latitude = longitude = None

    with ligar() as bd, bd.cursor() as c:
        # 2. O vaso existe? FOR UPDATE bloqueia a linha até ao fim: se duas pessoas tentarem
        #    registar o mesmo vaso ao mesmo tempo, a segunda espera pela primeira.
        c.execute(
            """SELECT id, utilizador_id, pin_hash, pin_expira_em, pin_tentativas
               FROM dispositivos WHERE codigo = %s FOR UPDATE""",
            (codigo,),
        )
        vaso = c.fetchone()
        if vaso is None:
            raise ErroAPI(403, "codigo_ou_pin_invalidos", MENSAGEM_PIN_INVALIDO)
        if vaso["utilizador_id"] is not None:
            raise ErroAPI(409, "vaso_ja_registado", "Este vaso já está registado.")
        if (vaso["pin_hash"] is None or vaso["pin_expira_em"] < agora
                or vaso["pin_tentativas"] >= MAX_TENTATIVAS_PIN):
            raise ErroAPI(403, "codigo_ou_pin_invalidos", MENSAGEM_PIN_INVALIDO)

        # 3. O PIN está certo? Se não, conta a tentativa e, à 5.ª, anula o PIN.
        if not pin_correto(pedido.pin, vaso["pin_hash"]):
            tentativas = vaso["pin_tentativas"] + 1
            if tentativas >= MAX_TENTATIVAS_PIN:
                c.execute(
                    "UPDATE dispositivos SET pin_hash = NULL, pin_expira_em = NULL, pin_tentativas = %s WHERE id = %s",
                    (MAX_TENTATIVAS_PIN, vaso["id"]),
                )
            else:
                c.execute("UPDATE dispositivos SET pin_tentativas = %s WHERE id = %s", (tentativas, vaso["id"]))
            bd.commit()                     # grava a tentativa ANTES do erro (senão o rollback apagava-a)
            raise ErroAPI(403, "codigo_ou_pin_invalidos", MENSAGEM_PIN_INVALIDO)

        # 4. A espécie existe?
        c.execute("SELECT id FROM especies WHERE id = %s", (pedido.especie_id,))
        if c.fetchone() is None:
            raise ErroAPI(400, "dados_invalidos", detalhes=[f"especie_id: a espécie {pedido.especie_id} não existe"])

        # 5. Cria o utilizador, ou atualiza o nome se o email já existir
        c.execute(
            """INSERT INTO utilizadores (email, nome) VALUES (%s, %s) AS novo
               ON DUPLICATE KEY UPDATE nome = novo.nome""",
            (pedido.email.strip(), pedido.nome_utilizador.strip()),
        )
        c.execute("SELECT id FROM utilizadores WHERE email = %s", (pedido.email.strip(),))
        id_utilizador = c.fetchone()["id"]

        # 6. Liga o vaso ao utilizador, guarda a localização e apaga o PIN (já não é preciso)
        c.execute(
            """UPDATE dispositivos
               SET utilizador_id = %s, cidade = %s, latitude = %s, longitude = %s,
                   pin_hash = NULL, pin_expira_em = NULL, pin_tentativas = 0
               WHERE id = %s""",
            (id_utilizador, pedido.cidade.strip(), latitude, longitude, vaso["id"]),
        )

        # 7. Cria a planta (por segurança, desativa outra que lá estivesse: só pode haver uma ativa)
        c.execute("UPDATE plantas SET ativa = FALSE WHERE dispositivo_id = %s AND ativa", (vaso["id"],))
        c.execute(
            "INSERT INTO plantas (nome, especie_id, dispositivo_id) VALUES (%s, %s, %s)",
            (pedido.nome_planta.strip(), pedido.especie_id, vaso["id"]),
        )
        id_planta = c.lastrowid

    return {"dispositivo": codigo, "planta_id": id_planta, "cidade": pedido.cidade.strip(),
            "latitude": latitude, "longitude": longitude}