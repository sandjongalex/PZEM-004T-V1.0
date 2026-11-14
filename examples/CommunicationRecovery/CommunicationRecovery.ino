// Detecte perte / retour de communication PZEM.
// IMPORTANT: un Timeout ne prouve pas a lui seul l'absence du 220 V.
// Uno: PZEM TX->D10, RX->D11, 5V->5V, GND->GND.
#include <SoftwareSerial.h>
#include <PZEM004T_V1.h>

SoftwareSerial pzemSerial(10, 11);
PZEM004T_V1 pzem(pzemSerial, 0xF8);

unsigned long lastRead = 0;
unsigned long lastSuccess = 0;
float lastEnergyKWh = NAN;
bool connected = false;
uint8_t failedReads = 0;

void setup() {
  Serial.begin(115200);
  pzem.begin(9600);
  Serial.println(F("Surveillance communication PZEM"));
}

void loop() {
  if (millis() - lastRead < 2000UL) return;
  lastRead = millis();

  if (pzem.read()) {
    failedReads = 0;
    lastSuccess = millis();
    lastEnergyKWh = pzem.energyKWh();
    if (!connected) {
      connected = true;
      Serial.println(F("PZEM: communication retablie"));
    }
    Serial.print(F("U=")); Serial.print(pzem.voltage(), 1);
    Serial.print(F(" V, E=")); Serial.print(lastEnergyKWh, 3);
    Serial.println(F(" kWh"));
  } else {
    if (failedReads < 255) ++failedReads;
    Serial.print(F("Lecture invalide: "));
    Serial.println(pzem.getErrorMessage());
    if (connected && failedReads >= 2) {
      connected = false;
      Serial.println(F("ALERTE: communication PZEM perdue"));
    }
    if (lastSuccess != 0) {
      Serial.print(F("Derniere energie valide (ancienne): "));
      Serial.print(lastEnergyKWh, 3);
      Serial.println(F(" kWh"));
    }
  }
}
