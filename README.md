# PZEM004T_V1 — Bibliothèque Arduino

**Version 0.1.0** — Bibliothèque Arduino destinée à lire les mesures électriques du module **PZEM-004T-100A-D-P V1.0** au moyen de **Modbus-RTU**. Cette version fournit les six grandeurs électriques, l'état de l'alarme et les erreurs de communication ; elle ne prend pas en charge plusieurs modules, la remise à zéro de l'énergie ni la modification de l'adresse Modbus du module.

## Matériel et protocole

- **Module :** PZEM-004T-100A-D-P V1.0.
- **Microcontrôleur :** ATmega328P (Arduino Uno, architecture `avr`).
- **Liaison :** UART logiciel via `SoftwareSerial`, **9600 bauds**.
- **Protocole :** Modbus-RTU, lecture des registres d'entrée par la fonction `0x04` (**10 registres** à partir de `0x0000`).
- **Adresse utilisée dans l'exemple :** `0x01`.

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

Respecter l'alimentation et les niveaux électriques spécifiés pour la version du module utilisée. Vérifier le brochage réel avant tout raccordement.

## Installation

**Installation manuelle :** copier le répertoire de la bibliothèque dans `Documents/Arduino/libraries/PZEM004T_V1/`, puis redémarrer l'IDE Arduino.

**Installation ZIP :** dans l'IDE Arduino, utiliser **Croquis → Inclure une bibliothèque → Ajouter la bibliothèque .ZIP...**, puis sélectionner l'archive de la bibliothèque.

## Exemple minimal

```cpp
#include <SoftwareSerial.h>
#include <PZEM004T_V1.h>

SoftwareSerial pzemSerial(10, 11); // RX, TX
PZEM004T_V1 pzem(pzemSerial, 0x01);

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

L'exemple complet se trouve dans [`examples/Monitoring/Monitoring.ino`](examples/Monitoring/Monitoring.ino). Ouvrir le moniteur série à **115200 bauds**.

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

## Feuille de route (non implémentée dans la version 0.1.0)

- **0.2 :** améliorations des erreurs et des délais.
- **0.3 :** gestion multi-PZEM.
- **0.4 :** remise à zéro de l'énergie.
- **1.0 :** documentation, tests et publication.

Ces éléments sont indiqués à titre de feuille de route, sans être inclus dans cette version.

## Sécurité électrique

**DANGER — TENSION SECTEUR.** Le PZEM est associé à une installation électrique reliée au secteur : **risque grave d'électrocution**. Toute intervention et tout câblage doivent être réalisés par une **personne qualifiée**, avec les protections adaptées et l'alimentation coupée lors de l'installation.

## Licence

Distribué sous licence [MIT](LICENSE) — © 2026 sandjongalex.
