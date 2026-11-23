#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"
#include "api.h"
#include "rede.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "Falta o secrets.h: copia o secrets.h.exemplo para secrets.h e preenche-o"
#endif

int apiEnviarLeitura(const DadosSensores& d, bool incluirAgua) {
  if (!redeLigada()) return -1;

  // Sem hora certa nao se envia: a leitura ficaria com a data errada (RF11).
  char dataHora[25];
  if (!horaISO(dataHora, sizeof(dataHora))) {
    Serial.println("API: ainda sem hora certa (NTP) - leitura nao enviada");
    return -2;
  }

  // Sem DHT valido nao se envia: temperatura e humidade do ar sao obrigatorias.
  if (!d.dhtTemValor) {
    Serial.println("API: sem leitura valida do DHT - leitura nao enviada");
    return -3;
  }

  // Montar o JSON. Os campos opcionais (luz, nivel_agua) so entram se existirem.
  char json[256];
  int n = snprintf(json, sizeof(json),
                   "{\"data_hora\":\"%s\",\"humidade_solo\":%d,"
                   "\"temperatura\":%.1f,\"humidade_ar\":%.1f",
                   dataHora, d.soloPct, d.temperatura, d.humidadeAr);
  if (d.luzPresente && d.luzOk) {
    n += snprintf(json + n, sizeof(json) - n, ",\"luz\":%ld", (long)d.luz);
  }
  if (incluirAgua) {
    n += snprintf(json + n, sizeof(json) - n, ",\"nivel_agua\":%d", d.aguaPct);
  }
  snprintf(json + n, sizeof(json) - n, "}");

  HTTPClient http;
  http.setTimeout(HTTP_TIMEOUT_MS);
  http.begin(String(API_URL) + "/api/leituras");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", API_KEY);

  Serial.print("API: POST /api/leituras ");
  Serial.println(json);

  int codigoHttp = http.POST((uint8_t*)json, strlen(json));
  String resposta = http.getString();
  http.end();

  switch (codigoHttp) {
    case 201: Serial.println("API: 201 leitura guardada"); break;
    case 200: Serial.println("API: 200 leitura ja existia (reenvio)"); break;
    case 400: Serial.print("API: 400 dados invalidos -> "); Serial.println(resposta); break;
    case 401: Serial.println("API: 401 chave errada ou em falta - verificar o API_KEY no secrets.h"); break;
    case 409: Serial.println("API: 409 o vaso ainda nao tem planta associada na BD"); break;
    default:
      Serial.print("API: falhou (");
      Serial.print(codigoHttp);
      Serial.println(") - tenta no proximo envio");
  }
  return codigoHttp;
}
