#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "ecra.h"
#include "rega.h"

#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define SCREEN_ADDRESS 0x3C

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
static bool ecraOk = false;

enum ecra { ECRA_SOLO, ECRA_AGUA, ECRA_AR, ECRA_LUZ, NUM_ECRAS };
static int ecraAtual = ECRA_SOLO;
static bool temLuz = false;   // so mostra o ecra da luz se houver sensor

void ecraIniciar() {
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  ecraOk = display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  if (!ecraOk) {
    Serial.println("OLED nao encontrado - a continuar sem ecra");
    return;
  }
  display.setTextColor(SSD1306_WHITE);
}

void ecraArranque(const char* codigoVaso) {
  if (!ecraOk) return;
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 15);
  display.println("PlantCare");
  display.setTextSize(1);
  display.setCursor(10, 45);
  display.print("Vaso ");
  display.println(codigoVaso);
  display.display();
}

void ecraProximo() {
  ecraAtual = (ecraAtual + 1) % NUM_ECRAS;
  if (ecraAtual == ECRA_LUZ && !temLuz) ecraAtual = ECRA_SOLO;
}

// Titulo em cima (com indicador de Wi-Fi), valor grande ao meio, estado em baixo.
static void desenhar(const char* titulo, const char* valor, const char* estado, bool wifi) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 5);
  display.print(titulo);
  display.setCursor(110, 5);
  display.print(wifi ? "W" : "-");   // W = ligado ao Wi-Fi
  display.drawLine(0, 18, 128, 18, SSD1306_WHITE);

  display.setTextSize(3);
  display.setCursor(10, 27);
  display.print(valor);

  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print(estado);

  display.display();
}

void ecraAtualizar(const DadosSensores& d, bool wifiLigado) {
  if (!ecraOk) return;
  temLuz = d.luzPresente;

  char valor[12];
  char estado[22];

  switch (ecraAtual) {
    case ECRA_SOLO: {
      snprintf(valor, sizeof(valor), "%d%%", d.soloPct);
      const char* s;
      if (regaBloqueada())                    s = "Verificar sensor!";
      else if (regaEstado() == BOMBA_A_REGAR) s = "A regar...";
      else if (d.soloPct < SOIL_START_PCT)    s = "Solo SECO";
      else if (d.soloPct < 70)                s = "Solo HUMIDO";
      else                                    s = "Solo MOLHADO";
      desenhar("HUMIDADE DO SOLO", valor, s, wifiLigado);
      break;
    }

    case ECRA_AGUA:
      snprintf(valor, sizeof(valor), "%d%%", d.aguaPct);
      desenhar("NIVEL DE AGUA", valor,
               (d.aguaPct < WATER_MIN_PCT) ? "Agua a acabar!" : "Reservatorio OK", wifiLigado);
      break;

    case ECRA_AR:
      if (!d.dhtTemValor) {
        desenhar("TEMPERATURA", "--", "Erro no DHT", wifiLigado);
      } else {
        snprintf(valor, sizeof(valor), "%.0fC", d.temperatura);
        snprintf(estado, sizeof(estado), "Humidade ar: %.0f%%", d.humidadeAr);
        desenhar("TEMPERATURA", valor, d.dhtOk ? estado : "Erro no DHT", wifiLigado);
      }
      break;

    case ECRA_LUZ:
      snprintf(valor, sizeof(valor), "%.0f", d.luz);
      desenhar("LUZ (lux)", valor, d.luzOk ? "" : "Erro no BH1750", wifiLigado);
      break;
  }
}
