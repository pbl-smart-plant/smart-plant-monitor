/*
* api.h - Comunicacao com a API REST (contrato-api.md, v0.4)
 */
#pragma once
#include "sensores.h"

// Envia uma leitura com POST /api/leituras.
// incluirAgua = true -> inclui "nivel_agua" (valor bruto), de hora a hora.
// Devolve o codigo HTTP (201, 200, 400, 401, 409, 500...) ou um valor
// negativo se nao houve resposta (sem rede, timeout, sem hora valida).
int apiEnviarLeitura(const DadosSensores& d, bool incluirAgua);
