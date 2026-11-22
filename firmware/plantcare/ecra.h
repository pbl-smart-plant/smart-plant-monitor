/*
* ecra.h - Ecra OLED SSD1306 128x64 (GME12864)
 * Se o ecra falhar, o resto do vaso continua a funcionar.
 */
#pragma once
#include "sensores.h"

void ecraIniciar();                                  // tambem faz o Wire.begin()
void ecraArranque(const char* codigoVaso);           // ecra inicial com o codigo do vaso
void ecraProximo();                                  // passa para o ecra seguinte
void ecraAtualizar(const DadosSensores& d, bool wifiLigado);
