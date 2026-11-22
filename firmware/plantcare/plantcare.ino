/*
 * PlantCare - Grupo 05 - Firmware do ESP32 (versao M2)
 *
 * Le a humidade do solo, o nivel de agua, a temperatura e humidade do ar
 * (DHT11) e a luz (BH1750, opcional); mostra tudo no OLED; rega por impulsos
 * sem precisar do servidor; e envia uma leitura a API a cada 5 minutos.
 *
 * Ficheiros:
 *   config.h        -> pinos, rele, calibracao e regras (comum as 4 replicas)
 *   config.local.h  -> valores proprios desta replica (opcional, fora do Git)
 *   secrets.h       -> Wi-Fi e chave da API (criar a partir do secrets.h.exemplo)
 *   sensores.*      -> leitura dos sensores
 *   rega.*          -> maquina de estados da bomba
 *   ecra.*          -> OLED
 *   rede.*          -> Wi-Fi, NTP e codigo do vaso
 *   api.*           -> POST /api/leituras
 *
 * Bibliotecas (Arduino IDE > Library Manager; no PlatformIO estao no platformio.ini):
 *   Adafruit SSD1306, Adafruit GFX, DHT sensor library (Adafruit),
 *   Adafruit Unified Sensor, BH1750 (Christopher Laws)
 */
#include "config.h"
#include "sensores.h"
#include "rega.h"
#include "ecra.h"
#include "rede.h"
#include "api.h"

unsigned long ultimaLeitura = 0;
unsigned long ultimoDHT = 0;
unsigned long ultimaLuz = 0;
unsigned long ultimaTrocaEcra = 0;
unsigned long ultimoEnvio = 0;
bool envioPendente = true;   // enviar logo que houver rede e hora certa
int  enviosFeitos = 0;

void imprimirEstado() {
  const DadosSensores& d = sensores();
  Serial.printf("Solo: %d%% (raw %d) | Agua: %d%% (raw %d) | Ar: ",
                d.soloPct, d.soloRaw, d.aguaPct, d.aguaRaw);
  if (d.dhtOk) Serial.printf("%.1f C, %.0f%%", d.temperatura, d.humidadeAr);
  else         Serial.print("erro");
  if (d.luzPresente) {
    if (d.luzOk) Serial.printf(" | Luz: %.0f lx", d.luz);
    else         Serial.print(" | Luz: erro");
  }
  Serial.printf(" | Bomba: %s | Impulsos: %d%s | WiFi: %s\n",
                regaEstadoTexto(), regaImpulsos(),
                regaBloqueada() ? " | BLOQUEADA" : "",
                redeLigada() ? "sim" : "nao");
}

void setup() {
  Serial.begin(115200);
  regaIniciar();          // primeiro: garante que a bomba arranca desligada
  ecraIniciar();          // faz o Wire.begin(), partilhado com o BH1750
  sensoresIniciar();

  Serial.println();
  Serial.print("=== PlantCare - vaso ");
  Serial.print(codigoVaso());
  Serial.println(" ===");
  ecraArranque(codigoVaso());

  redeIniciar();          // nao bloqueia: o vaso funciona mesmo sem Wi-Fi
  delay(2000);            // da tempo ao DHT11 para arrancar

  sensoresLerSoloAgua();  // primeiras leituras antes de decidir sobre a bomba
  sensoresLerDHT();
  sensoresLerLuz();
  unsigned long agora = millis();
  ultimaLeitura = ultimoDHT = ultimaLuz = ultimaTrocaEcra = ultimoEnvio = agora;
  imprimirEstado();
  ecraAtualizar(sensores(), redeLigada());
}

void loop() {
  unsigned long agora = millis();

  // 1) Sensores
  if (agora - ultimoDHT >= DHT_INTERVAL_MS) {
    ultimoDHT = agora;
    sensoresLerDHT();
  }
  if (agora - ultimaLuz >= LIGHT_INTERVAL_MS) {
    ultimaLuz = agora;
    sensoresLerLuz();
  }
  if (agora - ultimaLeitura >= READ_INTERVAL_MS) {
    ultimaLeitura = agora;
    sensoresLerSoloAgua();
    imprimirEstado();
    ecraAtualizar(sensores(), redeLigada());
  }

  // 2) Ecra
  if (agora - ultimaTrocaEcra >= SCREEN_INTERVAL_MS) {
    ultimaTrocaEcra = agora;
    ecraProximo();
    ecraAtualizar(sensores(), redeLigada());
  }

  // 3) Rega (sempre, com ou sem rede - RNF06)
  regaAtualizar(sensores().soloPct, sensores().aguaPct, millis());

  // 4) Rede
  redeAtualizar(agora);

  // 5) Envio a API a cada SEND_INTERVAL_MS
  if (agora - ultimoEnvio >= SEND_INTERVAL_MS) {
    ultimoEnvio = agora;
    envioPendente = true;
  }
  // Nunca enviar com a bomba a trabalhar: o pedido pode demorar ate 10 s
  // e o impulso ficaria mais longo do que devia (RF25).
  if (envioPendente && regaEstado() != BOMBA_A_REGAR && redeLigada() && horaValida()) {
    envioPendente = false;
    bool incluirAgua = (enviosFeitos % WATER_SEND_EVERY == 0);  // 1.o envio e depois de hora a hora
    int resultado = apiEnviarLeitura(sensores(), incluirAgua);
    if (resultado > 0) enviosFeitos++;
    // Se o servidor nao respondeu, esta leitura perde-se e segue a proxima,
    // daqui a SEND_INTERVAL_MS. O buffer offline de 24 h (RNF05) fica para
    // a entrega final.
  }
}
