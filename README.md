# PZEM004T_V1 — Bibliothèque Arduino

**Version 0.2.1** — Bibliothèque Arduino destinée à lire les mesures électriques du module **PZEM-004T-100A-D-P V1.0** au moyen de **Modbus-RTU**. Cette version fournit les six grandeurs électriques, l'état de l'alarme, la remise à zéro du compteur d'énergie par commande `0x42` et les erreurs de communication. Elle ne prend pas en charge plusieurs modules ni la modification de l'adresse Modbus du module.

## Matériel et protocole

- **Module :** PZEM-004T-100A-D-P V1.0.
- **Microcontrôleur :** ATmega328P (Arduino Uno, architecture `avr`).
- **Liaison :** UART logiciel via `SoftwareSerial`, **9600 bauds**.
- **Protocole :** Modbus-RTU, lecture des registres d'entrée par la fonction `0x04` (**10 registres** à partir de `0x0000`).
- **Adresse utilisée dans les exemples :** `0xF8`, adresse générale Modbus (un seul PZEM connecté). Si le module est configuré avec une adresse spécifique, cette adresse (`0x01` à `0xF7`) peut également être utilisée.
- **Attention aux noms de versions :** la carte `PZEM-004T-100A-D-P(V1.0)` utilise Modbus, contrairement à l'ancien PZEM-004T V1 historique qui utilise un protocole série différent.

## Mesures disponibles

| Grandeur | Méthode / champ | Unité |
| --- | --- | --- |
| Tension | `voltage()` / `PZEMData.voltage` | V |
| Courant | `current()` / `PZEMData.current` | A |
| Puissance active | `power()` / `PZEMData.power` | W |
| Énergie | `energyWh()` et `energyKWh()` / `PZEMData.energyWh` | Wh / kWh |
| Fréquence | `frequency()` / `PZEMData.frequency` | Hz |
| Facteur de puissance | `powerFactor()` / `PZEMData.powerFactor` | de 0 à 1 |
| Alarme | `alarm()` / `PZEMData.alarm` | `0xFFFF` = active ; autre valeur = inactive dans l'exemple |

Une lecture réussie renseigne `PZEMData.valid` à `true`. En cas d'échec, les mesures numériques sont invalidées (`NAN`) et `valid` passe à `false`.

## Câblage Arduino Uno ↔ PZEM

| PZEM-004T V1.0 | Arduino Uno |
| --- | --- |
| TX | D10 — RX de `SoftwareSerial` |
| RX | D11 — TX de `SoftwareSerial` |
| GND | GND — masse commune |
| 5V | 5V — alimentation de l'interface de communication TTL, si le module possède cette broche |

Pour la variante à 5 broches `5V / RX / TX / CF / GND`, ne pas raccorder `CF` pour ces exemples. Le module doit aussi être alimenté correctement côté mesure selon sa notice ; les broches 5V et GND **ne remplacent pas** l'alimentation du circuit de mesure.

Respecter l'alimentation et les niveaux électriques spécifiés pour la version du module utilisée. Vérifier le brochage réel avant tout raccordement.

## Installation

**Installation manuelle :** copier le répertoire de la bibliothèque dans `Documents/Arduino/libraries/PZEM004T_V1/`, puis redémarrer l'IDE Arduino.

**Installation ZIP :** dans l'IDE Arduino, utiliser **Croquis → Inclure une bibliothèque → Ajouter la bibliothèque .ZIP...**, puis sélectionner l'archive de la bibliothèque.

## Exemple minimal

```cpp
#include <SoftwareSerial.h>
#include <PZEM004T_V1.h>

SoftwareSerial pzemSerial(10, 11); // RX, TX
PZEM004T_V1 pzem(pzemSerial, 0xF8); // adresse generale, un seul PZEM

void setup() {
  Serial.begin(115200);
  pzem.begin(9600);
}

void loop() {
  if (pzem.read()) {
    Serial.print("Tension : ");
    Serial.println(pzem.voltage());
    Serial.print("Courant : ");
    Serial.println(pzem.current());
    Serial.print("Puissance : ");
    Serial.println(pzem.power());
    Serial.print("Energie (Wh) : ");
    Serial.println(pzem.energyWh());
    Serial.print("Energie (kWh) : ");
    Serial.println(pzem.energyKWh());
    Serial.print("Frequence : ");
    Serial.println(pzem.frequency());
    Serial.print("Facteur de puissance : ");
    Serial.println(pzem.powerFactor());
    Serial.print("Alarme : ");
    Serial.println(pzem.alarm() == 0xFFFF ? "ACTIVE" : "INACTIVE");
  } else {
    Serial.println(pzem.getErrorMessage());
  }
  delay(1000);
}
```

L'exemple de base se trouve dans [`examples/Monitoring/Monitoring.ino`](examples/Monitoring/Monitoring.ino). Ouvrir le moniteur série à **115200 bauds**.

## Exemples Arduino

Chaque croquis ci-dessous peut etre ouvert directement depuis le menu **Fichier > Exemples > PZEM004T_V1** d'Arduino IDE, apres installation de la bibliotheque.

| Exemple | Fonction | Bibliotheques externes |
| --- | --- | --- |
| [Monitoring](examples/Monitoring/Monitoring.ino) | Mesures electriques dans le moniteur serie | Aucune |
| [ResetEnergy](examples/ResetEnergy/ResetEnergy.ino) | Remise a zero manuelle de l'energie avec la commande **R** | Aucune |
| [LCD_I2C_16x2](examples/LCD_I2C_16x2/LCD_I2C_16x2.ino) | Affichage alternant tension/courant et puissance/energie | LiquidCrystal_I2C |
| [LCD_I2C_20x4](examples/LCD_I2C_20x4/LCD_I2C_20x4.ino) | Affichage simultane des quatre mesures principales | LiquidCrystal_I2C |
| [CSVSerial](examples/CSVSerial/CSVSerial.ino) | Export des mesures au format CSV, toutes les 2 secondes | Aucune |
| [CommunicationRecovery](examples/CommunicationRecovery/CommunicationRecovery.ino) | Detection de perte et de retour de la communication avec le PZEM | Aucune |
| [PowerAlarmBuzzer](examples/PowerAlarmBuzzer/PowerAlarmBuzzer.ino) | Alarme de depassement de puissance avec hysteresis et buzzer actif | Aucune |
| [DailyMonthlyDS3231](examples/DailyMonthlyDS3231/DailyMonthlyDS3231.ino) | Energie du jour et du mois avec DS3231, LCD 20x4, EEPROM | LiquidCrystal_I2C, RTClib (Adafruit) |
| [I2CScanner](examples/I2CScanner/I2CScanner.ino) | Detection de l'adresse I2C du LCD / DS3231 | Aucune |

### Installation des bibliotheques d'affichage et d'horloge

Pour les exemples LCD, installer **LiquidCrystal_I2C** (API avec constructeur `LiquidCrystal_I2C(0x27, 20, 4)`, `lcd.init()` et `lcd.backlight()`). Les implementations portant ce meme nom ne partagent pas toutes la meme API. L'exemple utilise le format de [johnrickman/LiquidCrystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C).

Pour `DailyMonthlyDS3231`, installer **RTClib by Adafruit** et les dependances proposees par le Gestionnaire de bibliotheques. `SoftwareSerial`, `Wire` et `EEPROM` sont fournis avec les cartes Arduino AVR.

Si le LCD reste vide, commencer par `I2CScanner` pour verifier son adresse (souvent `0x27`, parfois `0x3F`) puis ajuster `LiquidCrystal_I2C lcd(...)`.

### Brochage Arduino Uno

| Equipement | Broche | Arduino Uno |
| --- | --- | --- |
| PZEM Modbus | TX / RX | D10 (RX) / D11 (TX) |
| PZEM interface TTL | 5V / GND | 5V / GND |
| LCD I2C 16x2 ou 20x4 | SDA / SCL | A4 / A5 |
| DS3231 | SDA / SCL | A4 / A5 (bus partage avec le LCD) |
| Buzzer actif via transistor de commande | entree transistor | D8 |

Le 5V Arduino ne remplace **pas** l'alimentation necessaire au circuit de mesure du PZEM. Verifier la fiche technique du module et couper le secteur avant tout cablage. Le buzzer doit etre commande par un montage adapte a son courant, par exemple un transistor, et **n'est pas un dispositif de protection electrique**.

### Note sur le suivi journalier et mensuel

L'exemple `DailyMonthlyDS3231` utilise l'energie cumulee du PZEM pour estimer les deltas journaliers et mensuels. Il sauvegarde son etat en EEPROM toutes les 30 minutes et aux changements de date, et ne remet pas a zero le PZEM. **Regler l'horloge DS3231 avant utilisation** avec un exemple RTClib ; si l'heure est perdue, le programme s'arrete jusqu'a ce qu'elle soit reconfiguree.

Les consommations sont des **estimations**, notamment en cas de coupure/reinitialisation du compteur, d'absence de mesures au changement de jour/mois, ou de remise a zero du PZEM alors que l'Arduino est arrete. La variation traversant minuit n'est pas attribuee au nouveau jour. Ne pas utiliser cet exemple tel quel pour la facturation d'energie. Pour un produit final, prevoir un journal de mesures horodatees et un stockage non volatil adapte a l'endurance requise.

## Diagnostic en cas de Timeout

Si `pzem.read()` retourne `Timeout`, tester **d'abord** l'exemple `Monitoring` avec l'adresse générale `0xF8` avant toute remise à zéro d'énergie. Vérifier ensuite, alimentation coupée avant manipulation :

1. TX du PZEM connecté à RX de l'Arduino (D10) ; RX du PZEM connecté à TX de l'Arduino (D11).
2. 5V et GND de l'interface TTL correctement alimentés et reliés (l'interface ne peut pas communiquer sans eux).
3. Circuit de mesure du PZEM correctement alimenté selon la notice et les protections électriques applicables (l'USB de l'Arduino seul ne suffit pas à alimenter le circuit de mesure).
4. Vitesse `9600` bauds pour le PZEM ; moniteur série de l'IDE Arduino à `115200` bauds.
5. Si l'adresse `0xF8` ne fonctionne pas, confirmer le modèle exact et tester l'équipement et ses connexions avant de changer la bibliothèque.

Une indication `V1.0` sur la carte `PZEM-004T-100A-D-P(V1.0)` n'implique **pas** le protocole de l'ancienne génération V1 du PZEM-004T.

## Remise à zéro de l'énergie

La fonction `bool resetEnergy()` envoie la commande Modbus `0x42` au PZEM, contrôle le CRC et la réponse du module, puis met à jour la valeur d'énergie en mémoire **uniquement en cas de succès**. Elle remet à zéro le **compteur interne du module**, pas seulement l'affichage Arduino. Les autres mesures ne sont pas réinitialisées. Cette commande doit être prise en charge par le matériel installé (les anciennes variantes non-Modbus ne sont pas compatibles).

**Attention :** l'effacement est irréversible pour le compteur du PZEM. Sauvegarder l'énergie nécessaire à l'historique avant de l'exécuter ; ne pas appeler cette fonction automatiquement dans `setup()`.

```cpp
if (pzem.resetEnergy()) {
  Serial.println("Compteur d'energie remis a zero");
} else {
  Serial.println(pzem.getErrorMessage());
}
```

L'exemple [`examples/ResetEnergy/ResetEnergy.ino`](examples/ResetEnergy/ResetEnergy.ino) permet d'envoyer **R** depuis le moniteur série (115200 bauds) pour déclencher volontairement l'effacement. La réponse `0xC2` du module est traitée comme une erreur Modbus.

## Gestion des erreurs

| Valeur de `PZEMError` | Message fourni par la bibliothèque | Signification |
| --- | --- | --- |
| `PZEM_OK` | Aucune erreur | Dernière lecture réussie |
| `PZEM_ERROR_TIMEOUT` | Timeout | Aucune réponse dans le délai imparti |
| `PZEM_ERROR_CRC` | Erreur CRC | Contrôle CRC Modbus incorrect |
| `PZEM_ERROR_ADDRESS` | Adresse incorrecte | Adresse dans la réponse non conforme |
| `PZEM_ERROR_FUNCTION` | Fonction Modbus incorrecte | Code de fonction inattendu |
| `PZEM_ERROR_FRAME` | Trame incorrecte | Longueur ou structure de réponse invalide |
| `PZEM_ERROR_EXCEPTION` | Exception Modbus | Réponse d'exception du module |

La méthode `getError()` renvoie le code, et `getErrorMessage()` renvoie le texte correspondant.

## Feuille de route

- **0.2.0 :** remise à zéro de l'énergie via `resetEnergy()` (implémentée).
- **0.2.1 :** 7 nouveaux exemples Arduino (LCD, DS3231, CSV, diagnostic, alarme et scanner I2C).
- **0.3 :** améliorations des erreurs, des délais et tests complémentaires.
- **0.4 :** gestion multi-PZEM.
- **1.0 :** stabilisation et publication.

## Sécurité électrique

**DANGER — TENSION SECTEUR.** Le PZEM est associé à une installation électrique reliée au secteur : **risque grave d'électrocution**. Toute intervention et tout câblage doivent être réalisés par une **personne qualifiée**, avec les protections adaptées et l'alimentation coupée lors de l'installation.

## Licence

Distribué sous licence [MIT](LICENSE) — © 2026 sandjongalex.
