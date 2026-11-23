/*
 * config.h - Configuracao do PlantCare
 *
 * Valores comuns as 4 replicas. Os campos marcados com [ATENCAO] podem ter
 * de ser ajustados em cada casa.
 * As passwords e a chave da API NAO ficam aqui: vao para o secrets.h.
 */
#pragma once

// ---------- Pinos ----------
// Sensores analogicos em pinos do ADC1 (GPIO 32-39): continuam a funcionar
// com o Wi-Fi ligado (o ADC2 nao).
#define SOIL_PIN        34   // sinal (AOUT) do sensor de humidade do solo
#define WATER_PIN       35   // sinal (S) do sensor de nivel HW-038
#define WATER_POWER_PIN 25   // (+) do HW-038: so fica a 3,3 V durante a leitura
#define DHT_PIN         5    // dados do DHT11
#define PUMP_RELAY_PIN  23   // entrada (IN) do modulo rele
#define OLED_SDA_PIN    21   // I2C dados  (OLED e BH1750 partilham estes 2 pinos)
#define OLED_SCL_PIN    22   // I2C relogio

// Onde esta ligado o (+) do HW-038:
// true  = GPIO 25 (recomendado: o sensor so recebe corrente durante a leitura, corroi rapido)
#define WATER_POWER_CONTROL true

// ---------- Rele ----------
// REGRA DO GRUPO: a bomba liga-se SEMPRE aos terminais COM + NO do rele.
// [ATENCAO] Se, com a bomba no COM + NO, ela ligar ao contrario, trocar:
//   active-LOW:  RELAY_ON LOW,  RELAY_OFF HIGH
//   active-HIGH: RELAY_ON HIGH, RELAY_OFF LOW
#define RELAY_ON  HIGH
#define RELAY_OFF LOW

// ---------- Sensores ----------
#define DHT_TYPE DHT11           // DHT11 ou DHT22
#define BH1750_ADDRESS 0x23      // sensor de luz (opcional: detetado no arranque)

// ---------- Calibracao (valores brutos do ADC, 0-4095) ----------
// Valores "raw" do Monitor Serie, se forem bastante diferentes verificar e atualizar.
// [ATENCAO] Confirmar no monitor serie. Se a diferenca for grande
// (mais de ~10%), trocar pelos valores medidos na tua replica.
#define SOIL_DRY_RAW     2850    // sensor de solo ao ar          -> 0%
#define SOIL_WET_RAW     930     // sensor de solo dentro de agua -> 100%
#define WATER_EMPTY_RAW  0       // HW-038 seco                   -> 0%
#define WATER_FULL_RAW   1500    // HW-038 com as riscas submersas -> 100%

// ---------- Rega automatica (nivel 1: limites fixos, sem servidor) ----------
#define SOIL_START_PCT   30      // abaixo disto comeca um ciclo de rega
#define SOIL_STOP_PCT    50      // a partir disto o ciclo termina (histerese)
#define WATER_MIN_PCT    20      // abaixo disto a bomba nunca liga (RF26)
#define PUMP_PULSE_MS    3000UL  // duracao de cada impulso
#define SOAK_MS          60000UL // espera para a agua se espalhar pela terra
#define MAX_PULSES       3       // 3 x 3 s = 9 s por ciclo: cumpre o maximo de 10 s (RF25)

// ---------- Tempos ----------
#define READ_INTERVAL_MS    1000UL           // ler solo e agua
#define DHT_INTERVAL_MS     2000UL           // o DHT11 nao aceita leituras mais rapidas
#define LIGHT_INTERVAL_MS   2000UL           // ler o sensor de luz
#define SCREEN_INTERVAL_MS  3000UL           // trocar de ecra no OLED
#define SEND_INTERVAL_MS    (5UL * 60 * 1000) // enviar uma leitura a API a cada 5 min
#define WATER_SEND_EVERY    12               // enviar nivel_agua 1 vez em cada 12 envios (1 h)
#define NUM_SAMPLES         10               // leituras analogicas por media

// ---------- Rede ----------
#define WIFI_RETRY_MS       30000UL          // tentar de novo o Wi-Fi a cada 30 s
#define HTTP_TIMEOUT_MS     10000            // timeout de cada pedido (contrato, secao 4)
#define NTP_SERVER          "pool.ntp.org"

// ---------- Configuracao do Wi-Fi pelo telemovel (WiFiManager) ----------
#define WIFI_AP_NAME        "PlantCare-Setup" // rede criada pelo ESP32 para configurar o Wi-Fi
#define PORTAL_TIMEOUT_S    180               // fecha o portal ao fim de 3 min (so se ja houver rede guardada)
#define RESET_BUTTON_PIN    0                 // botao BOOT da placa
#define RESET_HOLD_MS       5000UL            // carregar 5 s no BOOT apaga a rede guardada

