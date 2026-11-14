// PZEM + LCD I2C 16x2 (LiquidCrystal_I2C, adresse typique 0x27).
// Arduino Uno: PZEM TX->D10, RX->D11, 5V->5V, GND->GND.
// LCD: SDA->A4, SCL->A5, VCC->5V, GND->GND.
// Adresse PZEM 0xF8: pour un seul module sur la liaison.
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <PZEM004T_V1.h>

SoftwareSerial pzemSerial(10, 11);
PZEM004T_V1 pzem(pzemSerial, 0xF8);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adapter a 0x3F si necessaire.

unsigned long previousRead = 0;

void clearLine(uint8_t row) {
  lcd.setCursor(0, row);
  lcd.print(F("                "));
  lcd.setCursor(0, row);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  pzem.begin(9600);
  lcd.print(F("Compteur PZEM"));
  lcd.setCursor(0, 1);
  lcd.print(F("Initialisation"));
}

void loop() {
  if (millis() - previousRead < 1000UL) return;
  previousRead = millis();

  if (!pzem.read()) {
    clearLine(0);
    lcd.print(F("PZEM: erreur"));
    clearLine(1);
    lcd.print(pzem.getErrorMessage());
    return; // Timeout != energie nulle.
  }

  // Deux pages: tension/courant puis puissance/energie.
  const bool firstPage = ((millis() / 4000UL) % 2UL == 0);
  clearLine(0);
  clearLine(1);
  if (firstPage) {
    lcd.setCursor(0, 0);
    lcd.print(F("U: "));
    lcd.print(pzem.voltage(), 1);
    lcd.print(F(" V"));
    lcd.setCursor(0, 1);
    lcd.print(F("I: "));
    lcd.print(pzem.current(), 3);
    lcd.print(F(" A"));
  } else {
    lcd.setCursor(0, 0);
    lcd.print(F("P: "));
    lcd.print(pzem.power(), 1);
    lcd.print(F(" W"));
    lcd.setCursor(0, 1);
    lcd.print(F("E: "));
    lcd.print(pzem.energyKWh(), 3);
    lcd.print(F(" kWh"));
  }
}
