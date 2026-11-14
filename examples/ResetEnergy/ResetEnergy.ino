#include <SoftwareSerial.h>
#include <PZEM004T_V1.h>

SoftwareSerial pzemSerial(10, 11); // D10 RX, D11 TX
PZEM004T_V1 pzem(pzemSerial, 0x01);

void setup() {
  Serial.begin(115200);
  pzem.begin(9600);
  Serial.println(F("PZEM - Remise a zero manuelle de l'energie"));
  Serial.println(F("Envoyer R dans le moniteur serie pour effacer le compteur."));
}

void loop() {
  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'R' || command == 'r') {
      if (pzem.resetEnergy()) {
        Serial.println(F("Compteur d'energie remis a zero."));
      } else {
        Serial.print(F("Echec de la remise a zero : "));
        Serial.println(pzem.getErrorMessage());
      }
    }
  }

  static unsigned long lastRead = 0;
  if (millis() - lastRead >= 1000) {
    lastRead = millis();
    if (pzem.read()) {
      Serial.print(F("Energie : "));
      Serial.print(pzem.energyKWh(), 3);
      Serial.println(F(" kWh"));
    } else {
      Serial.println(pzem.getErrorMessage());
    }
  }
}
