#include "config.h"

int contador = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);   // da tempo ao monitor para abrir
}

void loop() {
    Serial.print("PlantCare a funcionar! ");
    Serial.println(contador++);
    delay(1000);
}