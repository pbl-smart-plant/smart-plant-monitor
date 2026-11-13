#!/usr/bin/env bash
# =====================================================================
# SÓ NO TEU COMPUTADOR: apaga a BD local e cria-a de novo, com dados de teste.
# Uso (dentro da pasta servidor/):   ./mysql/dev/recriar-bd.sh
# =====================================================================
set -euo pipefail                      # para logo ao primeiro erro

cd "$(dirname "$0")/../.."             # vai para a pasta servidor/, esteja onde estiver
source .env                            # lê AMBIENTE, MYSQL_ROOT_PASSWORD, ...

# Proteção: na AWS o .env tem AMBIENTE=producao, e isto recusa-se a correr.
if [[ "${AMBIENTE:-}" != "dev" ]]; then
  echo "❌ AMBIENTE=${AMBIENTE:-?}. Este script só corre com AMBIENTE=dev (apagava dados reais!)."
  exit 1
fi

echo "🗑️  A apagar a BD local..."
docker compose down -v                 # -v apaga o volume (os dados). OK porque é dev!

echo "🚀 A arrancar o MySQL e a criar as tabelas (01-esquema.sql + 02-utilizadores-bd.sh)..."
docker compose up -d --wait mysql      # --wait espera pelo healthcheck

echo "🌱 A carregar os dados de teste..."
docker compose exec -T mysql mysql -uroot -p"$MYSQL_ROOT_PASSWORD" plantcare < mysql/dev/dados-teste.sql

echo "✅ BD recriada com dados de teste."