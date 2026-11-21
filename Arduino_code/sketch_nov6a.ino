#define SOIL_PIN 2
#define WATER_PIN 4
#define PUMP_RELAY_PIN 23
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

int sensorValue = 0;      // Variable to store the raw ADC value
int dryValue = 640;      // Replace with your measured dry soil ADC value
int wetValue = 342;  
int waterSensorMIN = 0;
int waterSensorMAX = 1770;
int waterSensorValue = 0;
bool showWaterScreen = false;
unsigned long lastScreenChange = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  digitalWrite(PUMP_RELAY_PIN, LOW);

  // Initialize OLED
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED initialization failed!");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(10, 20);
  display.println("Starting...");
  display.display();

  delay(2000);
}

void loop() {

  sensorValue = analogRead(SOIL_PIN);  // Read raw sensor value
  waterSensorValue = analogRead(WATER_PIN);
  int WaterSensorPercent = map(waterSensorValue, waterSensorMAX,waterSensorMIN , 100, 0);
  WaterSensorPercent = constrain(WaterSensorPercent, 0, 100); // Limit range
  int soilMoisturePercent = map(sensorValue, wetValue, dryValue, 100, 0);
  soilMoisturePercent = constrain(soilMoisturePercent, 0, 100); // Limit range

  if (soilMoisturePercent < 30 && WaterSensorPercent >= 20) {
    digitalWrite(PUMP_RELAY_PIN, HIGH);
  }
  else {
    digitalWrite(PUMP_RELAY_PIN, LOW);
  }

  if (millis() - lastScreenChange >= 3000) {
    showWaterScreen = !showWaterScreen;
    lastScreenChange = millis();
  }

  // Serial Monitor
  Serial.print("Soil Moisture: ");
  Serial.print(soilMoisturePercent);
  Serial.println("%");
  Serial.print("Water Sensor ADC: ");
  Serial.println(waterSensorValue);

  // OLED display
  display.clearDisplay();

  if (showWaterScreen) {
    display.setTextSize(1);
    display.setCursor(10, 5);
    display.println("WATER LEVEL");
    display.drawLine(0, 18, 128, 18, SSD1306_WHITE);

    display.setTextSize(3);
    display.setCursor(25, 27);
    display.print(WaterSensorPercent);
    display.println("%");

    display.setTextSize(1);
    display.setCursor(10, 55);
  }
  else {
    display.setTextSize(1);
    display.setCursor(10, 5);
    display.println("SOIL MOISTURE");

    display.drawLine(0, 18, 128, 18, SSD1306_WHITE);

    display.setTextSize(3);
    display.setCursor(25, 27);
    display.print(soilMoisturePercent);
    display.println("%");

    display.setTextSize(1);
    display.setCursor(10, 55);

    if (soilMoisturePercent < 30) {
      display.println("Soil is DRY!");
    }
    else if (soilMoisturePercent < 70) {
      display.println("Soil is MOIST");
    }
    else {
      display.println("Soil is WET!");
    }
  }

  display.display();
}



