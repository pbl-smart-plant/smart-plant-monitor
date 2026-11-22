/*
* rega.h - Rega automatica por impulsos (nivel 1: funciona sem servidor)
 *
 * Regras (RF24, RF25, RF26, RNF06):
 *  - comeca um ciclo quando o solo desce abaixo de SOIL_START_PCT;
 *  - rega por impulsos de PUMP_PULSE_MS, com SOAK_MS de espera entre eles;
 *  - termina quando o solo chega a SOIL_STOP_PCT (histerese);
 *  - no maximo MAX_PULSES impulsos por ciclo (9 s no total) e depois bloqueia;
 *  - sem agua no reservatorio (abaixo de WATER_MIN_PCT) a bomba desliga sempre.
 */
#pragma once

enum EstadoBomba { BOMBA_PARADA, BOMBA_A_REGAR, BOMBA_A_ABSORVER };

void regaIniciar();
void regaAtualizar(int soloPct, int aguaPct, unsigned long agora);  // chamar em todas as voltas do loop
EstadoBomba regaEstado();
const char* regaEstadoTexto();
bool regaBloqueada();
int  regaImpulsos();
