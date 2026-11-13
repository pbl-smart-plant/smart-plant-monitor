"""Meteorologia do Open-Meteo: de 3 em 3 horas, guarda a previsão para o sítio de cada vaso.
Dados meteorológicos: Open-Meteo.com (licença CC BY 4.0 — a atribuição é obrigatória).

Para correr à mão uma vez (útil para testar):
    python meteorologia.py
"""
import asyncio
import logging
import os
from datetime import datetime, timedelta, timezone

import httpx

from bd import ligar

URL_PREVISAO = os.getenv("OPEN_METEO_URL", "https://api.open-meteo.com/v1/forecast")
URL_GEOCODIFICACAO = os.getenv("OPEN_METEO_GEOCODING_URL", "https://geocoding-api.open-meteo.com/v1/search")

log = logging.getLogger("plantcare.meteo")


class CidadeNaoEncontrada(Exception):
    """O Open-Meteo respondeu, mas não conhece a cidade."""


def agora_utc() -> datetime:
    """Hora atual em UTC, sem segundos, no formato que o MySQL guarda (sem fuso)."""
    return datetime.now(timezone.utc).replace(tzinfo=None, second=0, microsecond=0)


def geocodificar(cidade: str) -> tuple:
    """Converte o nome de uma cidade em (latitude, longitude). Ex.: "Porto" → (41.14961, -8.61099)."""
    resposta = httpx.get(
        URL_GEOCODIFICACAO,
        params={"name": cidade, "count": 1, "language": "pt", "format": "json"},
        timeout=10,
    )
    resposta.raise_for_status()                       # erro HTTP (ex.: 500) → exceção
    resultados = resposta.json().get("results") or []
    if not resultados:
        raise CidadeNaoEncontrada(cidade)
    return round(resultados[0]["latitude"], 5), round(resultados[0]["longitude"], 5)


def obter_previsao(latitude: float, longitude: float) -> list:
    """Previsão para as próximas 24 h, de 3 em 3 h.

    Devolve uma lista de tuplos:
    (previsto_para, temperatura, humidade_ar, precipitacao_mm, prob_precipitacao)
    """
    resposta = httpx.get(
        URL_PREVISAO,
        params={
            "latitude": latitude,
            "longitude": longitude,
            "hourly": "temperature_2m,relative_humidity_2m,precipitation,precipitation_probability",
            "forecast_days": 2,
            "timezone": "UTC",
        },
        timeout=10,
    )
    resposta.raise_for_status()
    hora_a_hora = resposta.json()["hourly"]           # listas paralelas: time[i], temperature_2m[i], ...

    agora = agora_utc()
    linhas = []
    for i, texto in enumerate(hora_a_hora["time"]):   # ex.: "2026-11-12T15:00"
        instante = datetime.fromisoformat(texto)
        if agora < instante <= agora + timedelta(hours=24) and instante.hour % 3 == 0:
            linhas.append((
                instante,
                hora_a_hora["temperature_2m"][i],
                hora_a_hora["relative_humidity_2m"][i],
                hora_a_hora["precipitation"][i],
                hora_a_hora["precipitation_probability"][i],
            ))
    return linhas


def recolher_meteo() -> int:
    """Uma recolha completa: para cada vaso com cidade, guarda a previsão na tabela meteo.

    Devolve o número de linhas guardadas. Um vaso com erro não impede os outros.
    """
    obtido_em = agora_utc()
    with ligar() as bd, bd.cursor() as c:
        c.execute("SELECT id, cidade, latitude, longitude FROM dispositivos WHERE cidade IS NOT NULL")
        vasos = c.fetchall()

    total = 0
    for vaso in vasos:
        try:
            if vaso["latitude"] is None:
                # Acontece se o Open-Meteo estava em baixo quando o vaso foi registado:
                # tenta outra vez agora e guarda as coordenadas.
                latitude, longitude = geocodificar(vaso["cidade"])
                with ligar() as bd, bd.cursor() as c:
                    c.execute(
                        "UPDATE dispositivos SET latitude = %s, longitude = %s WHERE id = %s",
                        (latitude, longitude, vaso["id"]),
                    )
            else:
                latitude, longitude = float(vaso["latitude"]), float(vaso["longitude"])

            linhas = obter_previsao(latitude, longitude)
            with ligar() as bd, bd.cursor() as c:
                # INSERT IGNORE: se a mesma linha já existir (UNIQUE uq_meteo), ignora-a em vez de dar erro
                c.executemany(
                    """INSERT IGNORE INTO meteo
                       (dispositivo_id, obtido_em, previsto_para, temperatura,
                        humidade_ar, precipitacao_mm, prob_precipitacao)
                       VALUES (%s, %s, %s, %s, %s, %s, %s)""",
                    [(vaso["id"], obtido_em, *linha) for linha in linhas],
                )
            total += len(linhas)
        except Exception as erro:                       # rede, cidade desconhecida, BD...
            log.warning("Meteorologia falhou para o vaso %s: %s", vaso["id"], erro)

    log.info("Meteorologia: %d previsões guardadas para %d vasos", total, len(vasos))
    return total


async def ciclo_meteo():
    """Corre em segundo plano enquanto a API estiver ligada: recolhe já e depois de X em X horas."""
    horas = float(os.getenv("METEO_INTERVALO_HORAS", "3"))
    while True:
        try:
            # to_thread: a recolha usa pedidos "bloqueantes" (httpx, PyMySQL);
            # numa thread à parte, não pára a API enquanto espera pela internet.
            await asyncio.to_thread(recolher_meteo)
        except Exception:
            log.exception("Erro inesperado na recolha da meteorologia")
        await asyncio.sleep(horas * 3600)


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    print(f"Guardadas {recolher_meteo()} previsões.")