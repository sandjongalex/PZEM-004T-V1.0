// Compteur pedagogique: PZEM + DS3231 + LCD I2C 20x4 + sauvegarde EEPROM.
// Bibliotheques supplementaires: RTClib (Adafruit), LiquidCrystal_I2C.
// UNO PZEM: TX->D10, RX->D11, 5V->5V, GND->GND.
// UNO I2C (LCD et RTC sur le meme bus): SDA->A4, SCL->A5, VCC/GND.
// IMPORTANT: regler l'heure du DS3231 au prealable avec un exemple RTClib.
// Ce sketch N'EFFACE PAS l'energie physique du PZEM.
// Compteurs du jour et du mois: valeurs estimees depuis la premiere utilisation,
// remises a zero au changement de date et de mois (la RTC doit etre exacte).
// Sans lecture exactement a minuit, le delta franchissant minuit n'est pas
// affecte au nouveau jour. Si le mois change, le delta franchissant le mois
// est egalement ignore: ce n'est pas un compteur de facturation certifie.
// EEPROM: etat sauvegarde au changement de date et toutes les 30 minutes.
// Une remise a zero du PZEM pendant que l'Arduino est eteint peut perdre
// la consommation comprise entre la derniere lecture et cette remise a zero.
#include <SoftwareSerial.h>
#include <Wire.h>
#include <EEPROM.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <PZEM004T_V1.h>
#include <stddef.h>

SoftwareSerial pzemSerial(10, 11);
PZEM004T_V1 pzem(pzemSerial, 0xF8);
LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS3231 rtc;

const uint32_t STATE_MAGIC = 0x504A4D31UL;
const unsigned long SAVE_INTERVAL_MS = 1800000UL; // 30 minutes
const unsigned long READ_INTERVAL_MS = 1000UL;

struct MeterState {
  uint32_t magic;
  uint32_t dayKey;   // AAAAMMJJ
  uint32_t monthKey; // AAAAMM
  float lastWh;
  float dayWh;
  float monthWh;
  uint16_t checksum;
};

MeterState state = {};
bool stateValid = false;
unsigned long lastRead = 0;
unsigned long lastSave = 0;

uint16_t checksumState(const MeterState &s) {
  uint16_t crc = 0xFFFF;
  const uint8_t *bytes = (const uint8_t *)&s;
  for (size_t i = 0; i < offsetof(MeterState, checksum); ++i) {
    crc ^= bytes[i];
    for (uint8_t bit = 0; bit < 8; ++bit) {
      crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
    }
  }
  return crc;
}

void saveState() {
  state.magic = STATE_MAGIC;
  state.checksum = checksumState(state);
  EEPROM.put(0, state); // EEPROM.put utilise update sur Arduino AVR.
  lastSave = millis();
}

bool loadState() {
  EEPROM.get(0, state);
  return state.magic == STATE_MAGIC &&
         state.checksum == checksumState(state) &&
         state.monthKey == state.dayKey / 100UL &&
         (state.dayKey % 100UL) >= 1 &&
         (state.dayKey % 100UL) <= 31 &&
         (state.monthKey % 100UL) >= 1 &&
         (state.monthKey % 100UL) <= 12 &&
         !isnan(state.lastWh) && !isnan(state.dayWh) && !isnan(state.monthWh) &&
         !isinf(state.lastWh) && !isinf(state.dayWh) && !isinf(state.monthWh) &&
         state.lastWh >= 0 && state.dayWh >= 0 && state.monthWh >= 0;
}

void clearLine(uint8_t row) {
  lcd.setCursor(0, row);
  lcd.print(F("                    "));
  lcd.setCursor(0, row);
}

void printTwo(uint8_t value) {
  if (value < 10) lcd.print('0');
  lcd.print(value);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  pzem.begin(9600);

  if (!rtc.begin()) {
    lcd.print(F("RTC DS3231 absente"));
    Serial.println(F("RTC DS3231 absente. Verifier SDA/SCL."));
    while (true) delay(1000);
  }
  if (rtc.lostPower()) {
    lcd.print(F("Regler heure DS3231"));
    Serial.println(F("RTC non reglee: utiliser l'exemple ds3231 de RTClib."));
    // Pour regler UNE SEULE FOIS, televerser un sketch de configuration RTC
    // utilisant rtc.adjust(DateTime(F(__DATE__), F(__TIME__))).
    while (true) delay(1000);
  }

  stateValid = loadState();
  if (!stateValid) Serial.println(F("Nouveau suivi energie: pas d'historique EEPROM valide."));
}

void loop() {
  if (millis() - lastRead < READ_INTERVAL_MS) return;
  lastRead = millis();

  const DateTime now = rtc.now();
  const uint32_t dayKey = uint32_t(now.year()) * 10000UL +
                          uint32_t(now.month()) * 100UL + now.day();
  const uint32_t monthKey = dayKey / 100UL;

  if (!pzem.read()) {
    clearLine(0);
    lcd.print(F("PZEM: "));
    lcd.print(pzem.getErrorMessage());
    // Un Timeout n'est pas une energie nulle: ne pas modifier le suivi.
    return;
  }

  const float rawWh = pzem.energyWh();
  if (!stateValid) {
    state.magic = STATE_MAGIC;
    state.dayKey = dayKey;
    state.monthKey = monthKey;
    state.dayWh = 0;
    state.monthWh = 0;
    state.lastWh = rawWh;
    stateValid = true;
    saveState();
  } else {
    // Apres un reset physique du PZEM, le nouveau compteur repart de 0.
    // Dans ce cas, on n'invente pas d'energie negative.
    const float deltaWh = (rawWh >= state.lastWh)
                            ? (rawWh - state.lastWh) : rawWh;
    const bool newDay = (dayKey != state.dayKey);
    const bool newMonth = (monthKey != state.monthKey);

    if (newMonth) {
      state.monthWh = 0; // Delta du passage de mois inconnu.
    } else {
      state.monthWh += deltaWh;
    }
    if (newDay) {
      state.dayWh = 0; // Delta du passage de jour inconnu.
    } else {
      state.dayWh += deltaWh;
    }

    state.dayKey = dayKey;
    state.monthKey = monthKey;
    state.lastWh = rawWh;
    if (newDay || newMonth || millis() - lastSave >= SAVE_INTERVAL_MS) {
      saveState();
    }
  }

  clearLine(0);
  printTwo(now.day()); lcd.print('/'); printTwo(now.month());
  lcd.print(F(" U:")); lcd.print(pzem.voltage(), 1); lcd.print(F("V"));

  clearLine(1);
  lcd.print(F("Puissance: ")); lcd.print(pzem.power(), 1); lcd.print(F("W"));

  clearLine(2);
  lcd.print(F("Jour: ")); lcd.print(state.dayWh / 1000.0f, 3); lcd.print(F(" kWh"));

  clearLine(3);
  lcd.print(F("Mois: ")); lcd.print(state.monthWh / 1000.0f, 3); lcd.print(F(" kWh"));
}
