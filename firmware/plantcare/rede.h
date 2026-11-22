/*
* rede.h - Wi-Fi pelo portal WiFiManager (sem bloquear), hora certa por NTP
 *          e codigo do vaso
 */
#pragma once
#include <stddef.h>

void redeIniciar();
void redeAtualizar(unsigned long agora);   // chamar em todas as voltas do loop
bool redeLigada();
bool redePortalAtivo();                    // o portal "PlantCare-Setup" esta aberto?
bool horaValida();                         // o relogio ja foi acertado por NTP?
bool horaISO(char* destino, size_t tamanho); // ex.: "2026-11-22T10:00:00Z" (UTC)
const char* codigoVaso();                  // ex.: "PC-A3F1", a partir do ID do chip
