/*
* sensores.h - Leitura dos sensores: solo, agua, DHT11 e luz (opcional)
 */
#pragma once

struct DadosSensores {
    int   soloRaw  = 0;      // valor bruto do ADC (0-4095)
    int   soloPct  = 0;      // humidade do solo em %
    int   aguaRaw  = 0;      // valor bruto do HW-038 (e este que vai para a API)
    int   aguaPct  = 0;      // nivel de agua em %
    float temperatura = 0;   // graus Celsius
    float humidadeAr  = 0;   // %
    bool  dhtOk    = false;  // a ultima leitura do DHT foi valida?
    bool  dhtTemValor = false; // ja houve pelo menos uma leitura valida?
    bool  luzPresente = false; // o BH1750 foi encontrado no arranque?
    float luz      = 0;      // lux
    bool  luzOk    = false;  // a ultima leitura de luz foi valida?
};

void sensoresIniciar();
void sensoresLerSoloAgua();   // rapido: chamar a cada READ_INTERVAL_MS
void sensoresLerDHT();        // chamar a cada DHT_INTERVAL_MS
void sensoresLerLuz();        // chamar a cada LIGHT_INTERVAL_MS (nao faz nada sem sensor)
const DadosSensores& sensores();
