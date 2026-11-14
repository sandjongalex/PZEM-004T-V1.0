// PZEM + LCD I2C 20x4 (LiquidCrystal_I2C).
// Uno: PZEM TX->D10, RX->D11, 5V->5V, GND->GND.
// LCD: SDA->A4, SCL->A5, VCC->5V, GND->GND.
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <PZEM004T_V1.h>

SoftwareSerial pzemSerial(10, 11);
PZEM004T_V1 pzem(pzemSerial, 0xF8);
LiquidCrystal_I2C lcd(0x27, 20, 4); // Tester 0x3F si besoin.
unsigned long lastRead = 0;

void clearLine(uint8_t row) {
  lcd.setCursor(0, row);
  lcd.print(F("                    "));
  lcd.setCursor(0, row);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  pzem.begin(9600);
  lcd.print(F("Compteur energetique"));
}

void loop() {
  if (millis() - lastRead < 1000UL) return;
  lastRead = millis();

  if (!pzem.read()) {
    clearLine(0);
    lcd.print(F("PZEM: "));
    lcd.print(pzem.getErrorMessage());
    // Ne pas remplacer les dernieres mesures par des zeros.
    return;
  }

  clearLine(0);
  lcd.print(F("Tension : "));
  lcd.print(pzem.voltage(), 1);
  lcd.print(F(" V"));

  clearLine(1);
  lcd.print(F("Courant : "));
  lcd.print(pzem.current(), 3);
  lcd.print(F(" A"));

  clearLine(2);
  lcd.print(F("Puiss.  : "));
  lcd.print(pzem.power(), 1);
  lcd.print(F(" W"));

  clearLine(3);
  lcd.print(F("Energie : "));
  lcd.print(pzem.energyKWh(), 3);
  lcd.print(F(" kWh"));
}
