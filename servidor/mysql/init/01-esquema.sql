-- =====================================================================
-- PlantCare (grupo 05) — Esquema da base de dados
-- Baseado no diagrama ER desenvolvido para a BD
-- Todas as datas são guardadas em UTC.
-- =====================================================================

USE plantcare;

-- 1. UTILIZADORES — quem usa o sistema
CREATE TABLE utilizadores (
    id INT AUTO_INCREMENT PRIMARY KEY,
    email VARCHAR(255) NOT NULL,
    nome varchar(255) NOT NULL,
    papel ENUM('utilizador', 'admin') NOT NULL DEFAULT 'utilizador',
    crido_em DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- 2. DISPOSITIVOS — o vaso físico (ESP32 + sensores)
CREATE TABLE dispositivos (
    id INT AUTO_INCREMENT PRIMARY KEY,
    codigo VARCHAR(20) NOT NULL UNIQUE, -- gerado do ID do chip (ex.: PC-A3F1)
    chave_api_hash VARCHAR(255) NOT NULL,
    utilizador_id INT NULL,
    cidade VARCHAR(100),
    latitude DECIMAL(8,5),
    longitude DECIMAL(8,5),
    ultima_comunicacao DATETIME NULL,
    -- PIN de emparelhamento (RF03): o vaso mostra-o no portal e envia-o à API.
    pin_hash VARCHAR(255) NULL,
    pin_expira_em DATETIME NULL, -- válido 24 h
    pin_tentativas TINYINT NOT NULL DEFAULT 0, -- 5 erradas → o PIN é anulado
    criado_em DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (utilizador_id) REFERENCES utilizadores(id),
    CONSTRAINT chk_disp_lat CHECK (latitude BETWEEN -90 AND 90),
    CONSTRAINT chk_disp_lon CHECK (longitude BETWEEN -180 AND 180),
    CONSTRAINT chk_disp_pin CHECK (pin_tentativas BETWEEN 0 AND 5) -- 5 erradas → o PIN é anulado
);

-- 3. ESPECIES — a "biblioteca de plantas"
CREATE TABLE especies (
    id INT AUTO_INCREMENT PRIMARY KEY,
    nome VARCHAR(255) NOT NULL UNIQUE,
    humidade_solo_min DECIMAL(5,2) NOT NULL,
    humidade_solo_max DECIMAL(5,2) NOT NULL,
    temperatura_min DECIMAL(5,2) NOT NULL,
    temperatura_max DECIMAL(5,2) NOT NULL,
    humidade_ar_min DECIMAL(5,2) NOT NULL,
    humidade_ar_max DECIMAL(5,2) NOT NULL,
    luz_min INT NOT NULL,
    luz_max INT NOT NULL,
    CONSTRAINT chk_esp_solo CHECK ( humidade_solo_min BETWEEN 0 AND 100 AND
                                    humidade_solo_max BETWEEN 0 AND 100 AND
                                    humidade_solo_min <= humidade_solo_max),
    CONSTRAINT chk_esp_temp CHECK ( temperatura_min <= temperatura_max ),
    CONSTRAINT chk_esp_ar CHECK ( humidade_ar_min BETWEEN 0 AND 100 AND
                                  humidade_ar_max BETWEEN 0 AND 100 AND
                                  humidade_ar_min <= humidade_ar_max),
    CONSTRAINT chk_esp_luz CHECK ( luz_min >= 0 AND luz_min <= luz_max)
);

-- 4. plantas
CREATE TABLE plantas (
    id INT AUTO_INCREMENT PRIMARY KEY,
    nome VARCHAR(255) NOT NULL,
    especie_id INT NOT NULL,
    dispositivo_id INT NULL,
    ativa BOOLEAN NOT NULL DEFAULT TRUE,
    humidade_solo_min DECIMAL(5,2) NULL,
    humidade_solo_max DECIMAL(5,2) NULL,
    criada_em DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    -- Coluna calculada: igual ao dispositivo_id se a planta estiver ativa, senão NULL.
    -- Com o UNIQUE em baixo, garante que cada vaso tem NO MÁXIMO UMA planta ativa
    dispositivo_ativo INT AS (IF(ativa, dispositivo_id, NULL)) VIRTUAL,
    UNIQUE  KEY uq_uma_planta_ativa_por_vaso (dispositivo_ativo),
    FOREIGN KEY (especie_id) REFERENCES especies(id),
    FOREIGN KEY (dispositivo_id) REFERENCES dispositivos(id),
CONSTRAINT chk_planta_solo CHECK ( (humidade_solo_min IS NOT NULL OR
                                    humidade_solo_min BETWEEN 0 AND 100) AND
                                   (humidade_solo_max IS NOT NULL OR
                                    humidade_solo_max BETWEEN 0 AND 100) AND
                                   (humidade_solo_min IS NULL OR
                                    humidade_solo_max IS NULL OR
                                    humidade_solo_min <= humidade_solo_max))
);

-- 5. Leituras
CREATE TABLE leituras (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    planta_id INT NOT NULL,
    data_hora DATETIME NOT NULL,
    recebido_em DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    humidade_solo DECIMAL(5,2),
    temperatura DECIMAL(5,2),
    humidade_ar DECIMAL(5,2),
    luz INT,
    nivel_agua INT,
    FOREIGN KEY (planta_id) REFERENCES plantas(id),
    UNIQUE KEY uq_leitura (planta_id, data_hora),
    CONSTRAINT chk_leit_solo CHECK ( humidade_solo BETWEEN 0 AND 100),
    CONSTRAINT chk_leit_temp CHECK ( temperatura BETWEEN -20 and 60),
    CONSTRAINT chk_leit_ar CHECK ( humidade_ar BETWEEN 0 AND 100),
    CONSTRAINT chk_leit_luz CHECK ( luz >= 0 ),
    -- Porquê 4095 no nível de água? O ADC do ESP32 tem 12 bits: dá valores de 0 a 2¹² − 1 = 4095.
    CONSTRAINT chk_leit_agua CHECK ( nivel_agua BETWEEN 0 AND 4095)
);

-- 6. alertas
CREATE TABLE alertas (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    planta_id INT NOT NULL,
    tipo ENUM('humidade_baixa', 'humidade_alta', 'temperatura', 'luz', 'reservatorio_vazio') NOT NULL,
    mensagem VARCHAR(255) NOT NULL,
    criado_em DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    resolvido_em DATETIME NULL,
    FOREIGN KEY (planta_id) REFERENCES plantas(id),
    INDEX idx_alertas_planta_tipo_data (planta_id, tipo, criado_em)
);

-- 7. comandos (comandos dados no grafana para atuadores)
CREATE TABLE comandos (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    dispositivos_id INT NOT NULL,
    utilizador_id INT NULL,
    tipo ENUM('regar') NOT NULL DEFAULT 'regar',
    estado ENUM('pendente', 'executado', 'falhou', 'expirado') NOT NULL DEFAULT 'pendente',
    criado_em DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    executado_em DATETIME NULL,
    FOREIGN KEY (dispositivos_id) REFERENCES dispositivos(id),
    FOREIGN KEY (utilizador_id) REFERENCES  utilizadores(id),
    INDEX idx_comandos_disp_estado (dispositivos_id, estado)
);

-- 8. regas
CREATE TABLE regas (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    planta_id INT NOT NULL,
    comando_id BIGINT NULL UNIQUE,
    inicio DATETIME NOT NULL,
    duracao_seg INT NOT NULL DEFAULT 0,
    origem ENUM('automatica', 'manual') NOT NULL,
    resultado ENUM('feita', 'bloqueada_sem_agua') NOT NULL,
    FOREIGN KEY (planta_id) REFERENCES plantas(id),
    FOREIGN KEY (comando_id) REFERENCES comandos(id),
    INDEX idx_regas_planta_inicio (planta_id, inicio),
    CONSTRAINT chk_rega_duracao CHECK ( duracao_seg BETWEEN 0 AND 10)
);

-- 9. meteo
CREATE TABLE meteo (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    dispositivo_id INT NOT NULL,
    obtido_em DATETIME NOT NULL,
    previsto_para DATETIME NOT NULL,
    temperatura DECIMAL(5,2),
    humidade_ar DECIMAL(5,2),
    precipitacao_mm DECIMAL(6,2),
    prob_precipitacao INT,
    FOREIGN KEY (dispositivo_id) REFERENCES dispositivos(id),
    UNIQUE KEY uq_meteo (dispositivo_id, obtido_em, previsto_para),
    INDEX idx_meteo_disp_previsto (dispositivo_id, previsto_para),
    CONSTRAINT chk_meteo_prob CHECK ( prob_precipitacao BETWEEN 0 AND 100),
    CONSTRAINT chk_meteo_chuva CHECK ( precipitacao_mm >= 0 )
);

-- 10. previsoes -- IA
CREATE TABLE previsoes (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    planta_id INT NOT NULL,
    criado_em DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    proxima_rega DATETIME NOT NULL,
    versao_modelo VARCHAR(20) NOT NULL,
    FOREIGN KEY (planta_id) REFERENCES plantas(id),
    INDEX idx_previsoes_planta_data (planta_id, criado_em)
);

-- 11. Sugestoes
CREATE TABLE sugestoes (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    planta_id INT NOT NULL,
    texto VARCHAR(255) NOT NULL,
    urgencia TINYINT NOT NULL DEFAULT 3,
    criada_em DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    seguida BOOLEAN NULL,
    FOREIGN KEY (planta_id) REFERENCES plantas(id),
    INDEX idx_sugestoes_planta_data (planta_id, criada_em),
    CONSTRAINT chk_sug_urgencia CHECK ( urgencia BETWEEN 1 AND 3)
);

