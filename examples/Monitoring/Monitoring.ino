#include <SoftwareSerial.h>
#include "PZEM004T_V1.h"

#define PZEM_RX 10
#define PZEM_TX 11

SoftwareSerial pzemSerial(PZEM_RX, PZEM_TX);

PZEM004T_V1 pzem(pzemSerial, 0x01);

void setup() {

  Serial.begin(115200);

  pzem.begin(9600);

  Serial.println();
  Serial.println("================================");
  Serial.println("   PZEM004T V1.0 MONITORING");
  Serial.println("================================");
}

void loop() {

  if (pzem.read()) {

    PZEMData data = pzem.getData();

    Serial.println();
    Serial.println("----- MESURES ELECTRIQUES -----");

    Serial.print("Tension       : ");
    Serial.print(data.voltage, 1);
    Serial.println(" V");

    Serial.print("Courant       : ");
    Serial.print(data.current, 3);
    Serial.println(" A");

    Serial.print("Puissance     : ");
    Serial.print(data.power, 1);
    Serial.println(" W");

    Serial.print("Energie       : ");
    Serial.print(data.energyWh, 0);
    Serial.println(" Wh");

    Serial.print("Energie       : ");
    Serial.print(data.energyWh / 1000.0, 3);
    Serial.println(" kWh");

    Serial.print("Frequence     : ");
    Serial.print(data.frequency, 1);
    Serial.println(" Hz");

    Serial.print("Facteur PF    : ");
    Serial.println(data.powerFactor, 2);

    Serial.print("Alarme        : ");
    Serial.println(data.alarm == 0xFFFF ? "ACTIVE" : "INACTIVE");

    Serial.println("-------------------------------");

  } else {

    Serial.print("Erreur PZEM : ");
    Serial.println(pzem.getErrorMessage());
  }

  delay(1000);
}
