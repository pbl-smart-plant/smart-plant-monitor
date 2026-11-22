#!/usr/bin/env bash
# =====================================================================
# Cópia de segurança (backup) da BD do PlantCare. Corre NA VM DA AWS,
# todos os dias, pelo cron.
# Guarda as cópias em ~/backups (FORA do repositório) e apaga as que
# tiverem mais de 14 dias.
#
# À mão:   ./backup/backup-bd.sh      (dentro da pasta servidor/)
# =====================================================================
set -euo pipefail                      # para ao primeiro erro, também dentro de "a | b"

cd "$(dirname "$0")/.."                # vai para a pasta servidor/, esteja onde estiver
source <(tr -d '\r' < .env)       # lê o .env (o tr ignora fins de linha do Windows, se houver)

PASTA="$HOME/backups"
mkdir -p "$PASTA"
FICHEIRO="$PASTA/plantcare-$(date -u +%Y-%m-%d_%H%M).sql.gz"
PARCIAL="$FICHEIRO.parcial"            # escreve primeiro aqui; só no fim ganha o nome certo

# mysqldump: escreve toda a BD como SQL (CREATE TABLE + INSERT).
#   --single-transaction  → cópia consistente SEM bloquear a BD (a API continua a gravar leituras)
#   MYSQL_PWD             → a password vai numa variável, não na linha de comando
# gzip: comprime (o SQL de texto fica ~10× mais pequeno)
if ! docker compose exec -T -e MYSQL_PWD="$MYSQL_ROOT_PASSWORD" mysql \
       mysqldump -uroot --single-transaction plantcare | gzip > "$PARCIAL"; then
  rm -f "$PARCIAL"                     # apaga só a cópia falhada (nunca uma boa)
  echo "❌ $(date -u '+%F %T') Backup FALHOU"
  exit 1
fi

# Confirmar que o ficheiro não está vazio nem estragado
if [ ! -s "$PARCIAL" ] || ! gzip -t "$PARCIAL"; then
  rm -f "$PARCIAL"
  echo "❌ $(date -u '+%F %T') Backup vazio ou estragado"
  exit 1
fi
mv "$PARCIAL" "$FICHEIRO"              # só agora: a cópia está completa e verificada

# Apagar as cópias com mais de 14 dias (ficam sempre as 14 mais recentes)
find "$PASTA" -name 'plantcare-*.sql.gz' -mtime +13 -delete

echo "✅ $(date -u '+%F %T') Backup: $FICHEIRO ($(du -h "$FICHEIRO" | cut -f1))"
