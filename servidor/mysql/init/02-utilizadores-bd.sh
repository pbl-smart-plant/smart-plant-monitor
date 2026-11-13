#!/bin/bash
# =====================================================================
# Cria os utilizadores da BD com o MÍNIMO de permissões necessárias.
# Corre automaticamente na 1.ª vez, a seguir ao 01-esquema.sql.
# É um .sh (e não .sql) para ir buscar as passwords às variáveis de
# ambiente: assim nenhuma password fica escrita no repositório público.
# =====================================================================

mysql --protocol=socket -uroot -p"${MYSQL_ROOT_PASSWORD}" <<EOSQL
-- API: lê, insere e atualiza. NÃO pode apagar nem mudar tabelas.
CREATE USER 'plantcare_api'@'%' IDENTIFIED BY '${API_DB_PASSWORD}';
GRANT SELECT, INSERT, UPDATE ON plantcare.* TO 'plantcare_api'@'%';

-- Grafana: só lê.
CREATE USER 'grafana'@'%' IDENTIFIED BY '${GRAFANA_DB_PASSWORD}';
GRANT SELECT ON plantcare.* TO 'grafana'@'%';
EOSQL

# $? = resultado do último comando (0 = correu bem). Se falhou, para tudo:
# o contentor não arranca e o erro aparece em "docker compose logs mysql".
if [ $? -ne 0 ]; then
  echo "Erro ao criar os utilizadores da BD"
  exit 1
fi
echo "Utilizadores plantcare_api e grafana criados"