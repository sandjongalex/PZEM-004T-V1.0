// Export CSV des mesures PZEM via USB (115200 bauds).
// Copier les lignes dans un fichier .csv pour les analyser sur ordinateur.
// TX PZEM -> D10 ; RX PZEM -> D11 ; VCC/GND selon le module.
#include <SoftwareSerial.h>
#include <PZEM004T_V1.h>

SoftwareSerial pzemSerial(10, 11);
PZEM004T_V1 pzem(pzemSerial, 0xF8);
unsigned long lastRead = 0;

void setup() {
  Serial.begin(115200);
  pzem.begin(9600);
  Serial.println(F("time_ms,voltage_V,current_A,power_W,energy_Wh,frequency_Hz,power_factor"));
}

void loop() {
  if (millis() - lastRead < 2000UL) return;
  lastRead = millis();

  if (!pzem.read()) {
    Serial.print(F("# lecture impossible: "));
    Serial.println(pzem.getErrorMessage());
    return; // Ne pas ecrire de fausses mesures a 0.
  }

  Serial.print(millis()); Serial.print(',');
  Serial.print(pzem.voltage(), 1); Serial.print(',');
  Serial.print(pzem.current(), 3); Serial.print(',');
  Serial.print(pzem.power(), 1); Serial.print(',');
  Serial.print(pzem.energyWh(), 0); Serial.print(',');
  Serial.print(pzem.frequency(), 1); Serial.print(',');
  Serial.println(pzem.powerFactor(), 2);
}
