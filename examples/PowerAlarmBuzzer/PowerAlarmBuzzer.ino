// Alarme locale de puissance, buzzer ACTIF commande via transistor sur D8.
// Cet exemple n'est PAS une protection electrique et ne coupe PAS le secteur.
// Uno: PZEM TX->D10, RX->D11, alimentation TTL selon module.
// Transistor: commande depuis D8, buzzer alimente selon sa fiche technique.
#include <SoftwareSerial.h>
#include <PZEM004T_V1.h>

const uint8_t BUZZER_PIN = 8;
const float LIMIT_W = 1500.0f;
const float HYSTERESIS_W = 100.0f;

SoftwareSerial pzemSerial(10, 11);
PZEM004T_V1 pzem(pzemSerial, 0xF8);
bool alarmOn = false;
unsigned long lastRead = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  pzem.begin(9600);
}

void loop() {
  if (millis() - lastRead < 1000UL) return;
  lastRead = millis();

  if (!pzem.read()) {
    // Impossible d'evaluer la puissance: pas de conclusion sur la charge.
    digitalWrite(BUZZER_PIN, LOW);
    alarmOn = false;
    Serial.print(F("Lecture PZEM impossible: "));
    Serial.println(pzem.getErrorMessage());
    return;
  }

  const float watts = pzem.power();
  if (!alarmOn && watts >= LIMIT_W) alarmOn = true;
  if (alarmOn && watts <= LIMIT_W - HYSTERESIS_W) alarmOn = false;
  digitalWrite(BUZZER_PIN, alarmOn ? HIGH : LOW);

  Serial.print(F("Puissance="));
  Serial.print(watts, 1);
  Serial.print(F(" W, alarme="));
  Serial.println(alarmOn ? F("ACTIVE") : F("inactive"));
}
