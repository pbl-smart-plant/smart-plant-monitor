#include <Arduino.h>
#include "config.h"
#include "rega.h"

static EstadoBomba estado = BOMBA_PARADA;
static unsigned long estadoDesde = 0;
static bool cicloAtivo = false;   // estamos a meio de um ciclo de rega?
static bool bloqueada  = false;   // atingiu MAX_PULSES sem o solo ficar humido
static int  impulsos   = 0;

static void ligarBomba(bool ligar) {
  digitalWrite(PUMP_RELAY_PIN, ligar ? RELAY_ON : RELAY_OFF);
}

static void mudarEstado(EstadoBomba novo, unsigned long agora) {
  estado = novo;
  estadoDesde = agora;
}

void regaIniciar() {
  // Escrever RELAY_OFF antes do pinMode: a bomba nunca liga no arranque.
  digitalWrite(PUMP_RELAY_PIN, RELAY_OFF);
  pinMode(PUMP_RELAY_PIN, OUTPUT);
}

void regaAtualizar(int soloPct, int aguaPct, unsigned long agora) {
  // 1) Seguranca: sem agua, a bomba desliga sempre (RF26).
  if (aguaPct < WATER_MIN_PCT) {
    ligarBomba(false);
    mudarEstado(BOMBA_PARADA, agora);
    cicloAtivo = false;
    return;
  }

  // 2) Histerese: comeca abaixo de SOIL_START_PCT, termina em SOIL_STOP_PCT.
  if (soloPct >= SOIL_STOP_PCT) {
    cicloAtivo = false;
    bloqueada = false;  // o solo voltou a ficar humido: desbloqueia
  } else if (!cicloAtivo && !bloqueada && soloPct < SOIL_START_PCT) {
    cicloAtivo = true;
    impulsos = 0;
  }

  // 3) Rega por impulsos: regar -> esperar -> voltar a medir.
  switch (estado) {
    case BOMBA_PARADA:
      if (cicloAtivo) {
        if (impulsos >= MAX_PULSES) {
          // Sem resultado ao fim do maximo: sensor fora da terra ou avariado?
          bloqueada = true;
          cicloAtivo = false;
          Serial.println("AVISO: limite de impulsos atingido, rega bloqueada");
        } else {
          ligarBomba(true);
          impulsos++;
          mudarEstado(BOMBA_A_REGAR, agora);
        }
      }
      break;

    case BOMBA_A_REGAR:
      if (agora - estadoDesde >= PUMP_PULSE_MS) {
        ligarBomba(false);
        mudarEstado(BOMBA_A_ABSORVER, agora);
      }
      break;

    case BOMBA_A_ABSORVER:
      if (agora - estadoDesde >= SOAK_MS) {
        mudarEstado(BOMBA_PARADA, agora);
      }
      break;
  }
}

EstadoBomba regaEstado() { return estado; }
bool regaBloqueada()     { return bloqueada; }
int  regaImpulsos()      { return impulsos; }

const char* regaEstadoTexto() {
  switch (estado) {
    case BOMBA_A_REGAR:    return "A REGAR";
    case BOMBA_A_ABSORVER: return "A ABSORVER";
    default:               return "PARADA";
  }
}
