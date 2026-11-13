"""ligaçao à base de dados MySQL."""
import os
from contextlib import contextmanager
from pathlib import Path

import pymysql
from dotenv import load_dotenv

# No Mac, lê o servidor/.env (a pasta acima desta). No Docker não há .env:
# as variáveis vêm do docker-compose.yml, e o load_dotenv não as substitui.
load_dotenv(Path(__file__).resolve().parent.parent / ".env")

@contextmanager
def ligar():
    """Abre uma ligacao à BD e garante que no fim fica sempre fechada.

    Uso:
        with ligar() as bd, bd.cursor() as c:
            c.execute("SELECT...")

    se tudo correr bem, grava as alteracoes (commit).
    Se houver um erro, desfaz tudo o que ainda nao foi gravado (rollback).
    """
    ligacao = pymysql.connect(
        host=os.getenv("DB_HOST", "127.0.0.1"), # quando configurar o Docker passa a: "mysql"
        port=int(os.getenv("DB_PORT", "3306")),
        user=os.getenv("DB_USER", "plantcare_api"), # o utilizador SEM permissão de DELETE
        password=os.environ["API_DB_PASSWORD"],  # vem do .env (obrigatória)
        database=os.getenv("DB_NAME", "plantcare"),
        cursorclass=pymysql.cursors.DictCursor,  # cada linha vem como dicionário: linha["id"]
        autocommit=False,   # nós decidimos quando gravar
        init_command="SET time_zone = '+00:00'",  # todas as datas em UTC
    )
    try:
        yield ligacao
        ligacao.commit()
    except Exception:
        ligacao.rollback()
        raise
    finally:
        ligacao.close()