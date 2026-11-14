// Outil pratique pour trouver l'adresse I2C du LCD (souvent 0x27 ou 0x3F)
// et du DS3231 (habituellement 0x68). Moniteur serie: 115200 bauds.
// Aucun PZEM n'est necessaire pour ce diagnostic I2C.
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println(F("Scanner I2C"));
}

void loop() {
  uint8_t found = 0;
  for (uint8_t address = 0x03; address <= 0x77; ++address) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("Peripherique a 0x"));
      if (address < 16) Serial.print('0');
      Serial.println(address, HEX);
      ++found;
    }
  }
  if (found == 0) Serial.println(F("Aucun peripherique I2C detecte"));
  delay(5000);
}
