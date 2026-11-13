-- =====================================================================
-- PlantCare — DADOS FICTÍCIOS só para testar no computador de cada um.
-- Criado por IA.
-- ⚠️ NUNCA correr na AWS. Não está em mysql/init/, por isso não corre
--    sozinho: só quando alguém o manda correr à mão (recriar-bd.sh).
-- =====================================================================

USE plantcare;

-- "Agora", arredondado ao minuto (UTC). Todas as datas partem daqui.
SET @agora = DATE_FORMAT(UTC_TIMESTAMP(), '%Y-%m-%d %H:%i:00');

-- ---------------------------------------------------------------------
-- Utilizadores (o email tem de ser igual ao da conta no Grafana)
-- ---------------------------------------------------------------------
INSERT INTO utilizadores (id, email, nome, papel) VALUES
                        (1, 'admin@plantcare.pt',  'Administrador', 'admin'),
                        (2, 'daniel@plantcare.pt', 'Daniel',        'utilizador'),
                        (3, 'rute@plantcare.pt',   'Rute',          'utilizador'),
                        (4, 'rui@plantcare.pt',    'Rui',           'utilizador'),
                        (5, 'dylane@plantcare.pt', 'Dylane',        'utilizador');

-- ---------------------------------------------------------------------
-- Dispositivos: um vaso em casa de cada elemento + um por reclamar
-- O PC-0004 não comunica há 2 h (para testar o RF16).
-- ---------------------------------------------------------------------
INSERT INTO dispositivos (id, codigo, chave_api_hash, utilizador_id, cidade, latitude, longitude, ultima_comunicacao) VALUES
                      (1, 'PC-0001', 'hash-de-teste-1', 2,    'Lisboa',  38.72253, -9.13934, @agora),
                      (2, 'PC-0002', 'hash-de-teste-2', 3,    'Porto',   41.14961, -8.61099, @agora),
                      (3, 'PC-0003', 'hash-de-teste-3', 4,    'Coimbra', 40.20564, -8.41955, @agora),
                      (4, 'PC-0004', 'hash-de-teste-4', 5,    'Faro',    37.01869, -7.93044, @agora - INTERVAL 2 HOUR),
                      (5, 'PC-0005', 'hash-de-teste-5', NULL, NULL,      NULL,      NULL,     NULL);

-- O PC-0005 acabou de ser configurado: tem um PIN à espera de ser usado (RF03).
-- O hash é inventado: a sério, é a API que o calcula.
UPDATE dispositivos
SET pin_hash = 'hash-pin-de-teste', pin_expira_em = @agora + INTERVAL 20 HOUR
WHERE id = 5;

-- ---------------------------------------------------------------------
-- Espécies
-- ---------------------------------------------------------------------
INSERT INTO especies (id, nome, humidade_solo_min, humidade_solo_max, temperatura_min, temperatura_max,
                      humidade_ar_min, humidade_ar_max, luz_min, luz_max) VALUES
                      (1, 'Manjericão',    40, 60, 18, 28, 40, 70, 500, 1000),
                      (2, 'Cato',          10, 30, 15, 35, 10, 50, 600, 1000),
                      (3, 'Hortelã',       50, 70, 15, 25, 50, 80, 300,  800),
                      (4, 'Lírio-da-paz',  45, 65, 18, 27, 50, 80, 100,  500);

-- ---------------------------------------------------------------------
-- Plantas: uma ativa por vaso + uma antiga, já desativada, no PC-0001
-- ---------------------------------------------------------------------
INSERT INTO plantas (id, nome, especie_id, dispositivo_id, ativa, humidade_solo_min) VALUES
                     (1, 'Manjericão da cozinha', 1, 1, TRUE,  NULL),
                     (2, 'Cato da varanda',       2, 2, TRUE,  NULL),
                     (3, 'Hortelã do Rui',        3, 3, TRUE,  55),     -- limite próprio (extra, sem RF)
                     (4, 'Lírio da sala',         4, 4, TRUE,  NULL),
                     (5, 'Salsa (morreu 🥀)',     1, 1, FALSE, NULL);   -- inativa: não conta para o UNIQUE

-- ---------------------------------------------------------------------
-- Uma semana de leituras a cada 15 min para as 4 plantas ativas
-- A humidade DESCE devagar (a terra seca) e SOBE de repente de 2 em 2 dias (rega).
-- ---------------------------------------------------------------------
INSERT INTO leituras (planta_id, data_hora, humidade_solo, temperatura, humidade_ar, luz, nivel_agua)
WITH RECURSIVE t(n) AS (
    SELECT 0 UNION ALL SELECT n + 1 FROM t WHERE n < 671          -- 7 dias × 24 h × 4 = 672
),
               params AS (                                                      -- comportamento de cada planta
                   SELECT 1 AS planta_id, 65 AS hum_topo, 0.15 AS descida, 22 AS temp_base UNION ALL
                   SELECT 2, 32, 0.12, 24 UNION ALL
                   SELECT 3, 75, 0.13, 20 UNION ALL
                   SELECT 4, 68, 0.14, 23
               ),
               base AS (
                   SELECT p.*, t.n,
                          @agora - INTERVAL t.n * 15 MINUTE AS dh,
                          MOD(671 - t.n, 192) AS desde_rega -- passos de 15 min desde a última rega
                   FROM params p CROSS JOIN t
                   WHERE NOT (p.planta_id = 4 AND t.n < 8)                        -- PC-0004 calado há 2 h
               )
SELECT planta_id,
       dh,
       ROUND(LEAST(100, GREATEST(0, hum_topo - desde_rega * descida + (RAND() - 0.5) * 2)), 2),
       ROUND(temp_base + 4 * SIN((HOUR(dh) - 9) * PI() / 12) + (RAND() - 0.5), 2),
       ROUND(LEAST(100, GREATEST(0, 60 - 10 * SIN((HOUR(dh) - 9) * PI() / 12) + (RAND() - 0.5) * 4)), 2),
       GREATEST(0, FLOOR(900 * SIN((HOUR(dh) - 6) * PI() / 12) + RAND() * 50)),
       IF(MOD(n, 4) = 0, 2600 - (671 - n), NULL)                 -- nível de água só de hora a hora
FROM base;

-- ---------------------------------------------------------------------
-- Regas automáticas, nos momentos em que a humidade "salta"
-- (desde_rega = 0 → n = 671, 479, 287, 95). A última do lírio foi bloqueada.
-- ---------------------------------------------------------------------
INSERT INTO regas (planta_id, inicio, duracao_seg, origem, resultado)
WITH momentos(n) AS (SELECT 671 UNION ALL SELECT 479 UNION ALL SELECT 287 UNION ALL SELECT 95)
SELECT p.id,
       @agora - INTERVAL m.n * 15 MINUTE,
       IF(p.id = 4 AND m.n = 95, 0, 8),
       'automatica',
       IF(p.id = 4 AND m.n = 95, 'bloqueada_sem_agua', 'feita')
FROM plantas p CROSS JOIN momentos m
WHERE p.ativa = TRUE;

-- ---------------------------------------------------------------------
-- Comandos (botão de rega manual) — um de cada estado
-- ---------------------------------------------------------------------
INSERT INTO comandos (id, dispositivos_id, utilizador_id, estado, criado_em, executado_em) VALUES
                  (1, 1, 2, 'executado', @agora - INTERVAL 1 DAY,    @agora - INTERVAL 1 DAY + INTERVAL 20 SECOND),
                  (2, 4, 5, 'falhou',    @agora - INTERVAL 23 HOUR,  @agora - INTERVAL 23 HOUR + INTERVAL 25 SECOND),
                  (3, 4, 5, 'expirado',  @agora - INTERVAL 3 HOUR,   NULL),
                  (4, 2, 3, 'pendente',  @agora - INTERVAL 1 MINUTE, NULL);

-- Só o comando 1 (executado) originou uma rega
INSERT INTO regas (planta_id, comando_id, inicio, duracao_seg, origem, resultado) VALUES
    (1, 1, @agora - INTERVAL 1 DAY + INTERVAL 20 SECOND, 5, 'manual', 'feita');

-- ---------------------------------------------------------------------
-- Alertas
-- ---------------------------------------------------------------------
INSERT INTO alertas (planta_id, tipo, mensagem, criado_em, resolvido_em) VALUES
                 (1, 'humidade_baixa',     'Humidade do solo abaixo de 40%',  @agora - INTERVAL 2 DAY,  @agora - INTERVAL 2 DAY + INTERVAL 3 HOUR),
                 (4, 'reservatorio_vazio', 'Reservatório de água vazio',      @agora - INTERVAL 1 DAY,  NULL),
                 (3, 'humidade_baixa',     'Humidade do solo abaixo de 55%',  @agora - INTERVAL 5 HOUR, NULL);

-- ---------------------------------------------------------------------
-- Meteorologia: última recolha há 1 h, previsão para as próximas 24 h (de 3 em 3 h)
-- ---------------------------------------------------------------------
INSERT INTO meteo (dispositivo_id, obtido_em, previsto_para, temperatura, humidade_ar, precipitacao_mm, prob_precipitacao)
WITH RECURSIVE h(k) AS (SELECT 1 UNION ALL SELECT k + 1 FROM h WHERE k < 8)
SELECT d.id,
       @agora - INTERVAL 1 HOUR,
       @agora + INTERVAL k * 3 HOUR,
       ROUND(15 + RAND() * 8, 2),
       ROUND(55 + RAND() * 30, 2),
       ROUND(IF(RAND() < 0.3, RAND() * 4, 0), 2),
       FLOOR(RAND() * 100)
FROM dispositivos d CROSS JOIN h
WHERE d.latitude IS NOT NULL;

-- ---------------------------------------------------------------------
-- Previsões da IA e sugestões
-- ---------------------------------------------------------------------
INSERT INTO previsoes (planta_id, criado_em, proxima_rega, versao_modelo) VALUES
                  (1, @agora - INTERVAL 30 MINUTE, @agora + INTERVAL 20 HOUR, 'v0-teste'),
                  (2, @agora - INTERVAL 30 MINUTE, @agora + INTERVAL 30 HOUR, 'v0-teste'),
                  (3, @agora - INTERVAL 30 MINUTE, @agora + INTERVAL 6 HOUR,  'v0-teste'),
                  (4, @agora - INTERVAL 30 MINUTE, @agora + INTERVAL 18 HOUR, 'v0-teste');

INSERT INTO sugestoes (planta_id, texto, urgencia, criada_em, seguida) VALUES
                   (4, 'Encher o reservatório de água', 1, @agora - INTERVAL 1 DAY, NULL),
                   (3, 'Regar nas próximas 6 horas', 2, @agora - INTERVAL 30 MINUTE, NULL),
                   (1, 'Mudar o vaso para um sítio com mais luz', 3, @agora - INTERVAL 3 DAY, TRUE);