"""Script do ADMIN: cria um vaso na BD (ou dá-lhe uma chave nova) e mostra a chave UMA vez.

Uso (dentro da pasta servidor/api, com o ambiente virtual ativo):
    python criar_dispositivo.py PC-A3F1              # vaso novo
    python criar_dispositivo.py PC-A3F1 --nova-chave # o vaso já existe: chave nova (a antiga deixa de funcionar)

A chave só aparece aqui. Na BD fica apenas o hash.
Envia-a ao dono do vaso por mensagem PRIVADA.
"""
import argparse
import sys

import pymysql

from bd import ligar
from seguranca import gerar_chave, hash_chave

parser = argparse.ArgumentParser(description="Cria um vaso PlantCare e gera a sua chave secreta.")
parser.add_argument("codigo", help="código do vaso, ex.: PC-A3F1 (aparece no Monitor Série)")
parser.add_argument("--nova-chave", action="store_true",
                    help="o vaso já existe: substitui a chave (por exemplo, se a anterior se perdeu)")
args = parser.parse_args()

codigo = args.codigo.strip().upper()
chave = gerar_chave()

with ligar() as bd, bd.cursor() as c:
    if args.nova_chave:
        linhas = c.execute("UPDATE dispositivos SET chave_api_hash = %s WHERE codigo = %s",
                           (hash_chave(chave), codigo))
        if linhas == 0:
            sys.exit(f"Não existe nenhum vaso {codigo}. Para o criar, corre sem --nova-chave.")
    else:
        try:
            c.execute("INSERT INTO dispositivos (codigo, chave_api_hash) VALUES (%s, %s)",
                      (codigo, hash_chave(chave)))
        except pymysql.err.IntegrityError:
            sys.exit(f"Já existe um vaso {codigo}. Para lhe dar uma chave nova, junta --nova-chave.")

print(f"Vaso {codigo} {'com chave nova' if args.nova_chave else 'criado'}.")
print(f"Chave: {chave}")
print("Copia-a AGORA: não volta a ser mostrada. Envia-a ao dono do vaso por mensagem privada.")