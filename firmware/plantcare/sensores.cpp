#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>
#include "config.h"
#include "sensores.h"

static DHT dht(DHT_PIN, DHT_TYPE);
static BH1750 sensorLuz(BH1750_ADDRESS);
static DadosSensores dados;

// Media de varias leituras para reduzir o ruido.
static int lerMedia(int pino) {
  long total = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    total += analogRead(pino);
    delay(2);
  }
  return total / NUM_SAMPLES;
}

// Converte um valor bruto em % (raw0 -> 0%, raw100 -> 100%).
static int paraPercentagem(int raw, int raw0, int raw100) {
  return constrain(map(raw, raw0, raw100, 0, 100), 0, 100);
}

void sensoresIniciar() {
  if (WATER_POWER_CONTROL) {
    pinMode(WATER_POWER_PIN, OUTPUT);
    digitalWrite(WATER_POWER_PIN, LOW);
  }
  dht.begin();

  // O BH1750 e opcional: se nao responder no I2C, o vaso funciona sem luz
  // e o campo "luz" simplesmente nao e enviado a API.
  // (O Wire.begin() ja foi feito pelo ecra, que partilha o I2C.)
  dados.luzPresente = sensorLuz.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  Serial.println(dados.luzPresente ? "Sensor de luz BH1750 encontrado"
                                   : "Sem sensor de luz (opcional) - a continuar sem ele");
}

void sensoresLerSoloAgua() {
  dados.soloRaw = lerMedia(SOIL_PIN);

  if (WATER_POWER_CONTROL) {
    digitalWrite(WATER_POWER_PIN, HIGH);  // liga o HW-038
    delay(10);                            // espera o sinal estabilizar
  }
  dados.aguaRaw = lerMedia(WATER_PIN);
  if (WATER_POWER_CONTROL) {
    digitalWrite(WATER_POWER_PIN, LOW);   // desliga para nao corroer
  }

  dados.soloPct = paraPercentagem(dados.soloRaw, SOIL_DRY_RAW, SOIL_WET_RAW);
  dados.aguaPct = paraPercentagem(dados.aguaRaw, WATER_EMPTY_RAW, WATER_FULL_RAW);
}

void sensoresLerDHT() {
  float t = dht.readTemperature();  // Celsius
  float h = dht.readHumidity();
  // Se falhar, a biblioteca devolve NAN: guardamos os ultimos valores bons.
  if (isnan(t) || isnan(h)) {
    dados.dhtOk = false;
    return;
  }
  dados.temperatura = t;
  dados.humidadeAr  = h;
  dados.dhtOk = true;
  dados.dhtTemValor = true;
}

void sensoresLerLuz() {
  if (!dados.luzPresente) return;
  float lux = sensorLuz.readLightLevel();   // negativo = erro
  if (lux < 0) {
    dados.luzOk = false;
    return;
  }
  dados.luz = lux;
  dados.luzOk = true;
}

const DadosSensores& sensores() {
  return dados;
}
