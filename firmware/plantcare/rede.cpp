#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <time.h>
#include "config.h"
#include "rede.h"

// O Wi-Fi e configurado pelo telemovel (portal do WiFiManager) e fica
// guardado na memoria interna do ESP32. Nao ha passwords no codigo (RNF10).
static WiFiManager wm;

static char codigo[16] = "";
static unsigned long ultimaTentativa = 0;
static unsigned long botaoDesde = 0;   // ha quanto tempo o BOOT esta carregado (0 = solto)
static bool estavaLigado = false;

const char* codigoVaso() {
  if (codigo[0] == '\0') {
    uint64_t chipId = ESP.getEfuseMac();   // unico em cada ESP32
    snprintf(codigo, sizeof(codigo), "PC-%04X", (uint16_t)(chipId >> 32));
  }
  return codigo;
}

void redeIniciar() {
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);   // botao BOOT: carregado = LOW

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  // Modo NAO bloqueante: com o portal aberto, o loop() continua a correr
  // (sensores, OLED e rega nunca param). O portal e tratado no redeAtualizar().
  wm.setConfigPortalBlocking(false);
  wm.setConnectTimeout(20);                  // espera ate 20 s pela rede guardada
  wm.setTitle("PlantCare");

  if (wm.getWiFiIsSaved()) {
    // Ja ha uma rede guardada: se ela falhar no arranque (router desligado),
    // o portal fecha ao fim de algum tempo e o vaso volta a tentar a rede.
    wm.setConfigPortalTimeout(PORTAL_TIMEOUT_S);
  }
  // Sem rede guardada (primeira vez): o portal fica aberto ate ser configurado.

  if (wm.autoConnect(WIFI_AP_NAME)) {
    Serial.println("Wi-Fi: ligado a rede guardada");
  } else {
    Serial.println("Wi-Fi: portal de configuracao aberto");
    Serial.print("  No telemovel, liga-te a rede \"");
    Serial.print(WIFI_AP_NAME);
    Serial.println("\" e escolhe a rede da casa.");
  }

  ultimaTentativa = millis();
  configTime(0, 0, NTP_SERVER);              // UTC; acerta o relogio quando houver rede
}

// Botao BOOT carregado durante RESET_HOLD_MS com o vaso a funcionar:
// apaga a rede guardada e reinicia, para abrir o portal de novo.
static void verificarBotaoReset(unsigned long agora) {
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    if (botaoDesde == 0) botaoDesde = agora;
    if (agora - botaoDesde >= RESET_HOLD_MS) {
      Serial.println("Botao BOOT: a apagar a rede Wi-Fi guardada e a reiniciar...");
      wm.resetSettings();
      delay(500);
      ESP.restart();
    }
  } else {
    botaoDesde = 0;
  }
}

void redeAtualizar(unsigned long agora) {
  wm.process();                              // trata o portal (se estiver aberto)
  verificarBotaoReset(agora);

  bool ligado = (WiFi.status() == WL_CONNECTED);
  if (ligado && !estavaLigado) {
    Serial.print("Wi-Fi ligado a \"");
    Serial.print(WiFi.SSID());
    Serial.print("\". IP: ");
    Serial.println(WiFi.localIP());
  } else if (!ligado && estavaLigado) {
    Serial.println("Wi-Fi perdido - o vaso continua a funcionar offline");
  }
  estavaLigado = ligado;

  // Sem ligacao e sem portal aberto: tenta a rede guardada a cada
  // WIFI_RETRY_MS, sem bloquear.
  if (!ligado && !wm.getConfigPortalActive() && wm.getWiFiIsSaved() &&
      agora - ultimaTentativa >= WIFI_RETRY_MS) {
    ultimaTentativa = agora;
    WiFi.reconnect();
  }
}

bool redeLigada() {
  return WiFi.status() == WL_CONNECTED;
}

bool redePortalAtivo() {
  return wm.getConfigPortalActive();
}

bool horaValida() {
  // Antes do NTP, o relogio do ESP32 comeca em 1970.
  return time(nullptr) > 1700000000;   // depois de novembro de 2023
}

bool horaISO(char* destino, size_t tamanho) {
  if (!horaValida()) return false;
  time_t agora = time(nullptr);
  struct tm utc;
  gmtime_r(&agora, &utc);
  strftime(destino, tamanho, "%Y-%m-%dT%H:%M:%SZ", &utc);
  return true;
}
