#include "PZEM004T_V1.h"

#define PZEM_TIMEOUT 500

#define MODBUS_READ_INPUT 0x04

#define PZEM_FIRST_REGISTER 0x0000
#define PZEM_REGISTER_COUNT 10

PZEM004T_V1::PZEM004T_V1(
  SoftwareSerial &serial,
  uint8_t address
) {

  _serial = &serial;
  _address = address;

  _error = PZEM_OK;

  clearData();
}

void PZEM004T_V1::begin(uint32_t baudrate) {

  _serial->begin(baudrate);
}

void PZEM004T_V1::clearData() {

  _data.voltage = NAN;
  _data.current = NAN;
  _data.power = NAN;
  _data.energyWh = NAN;
  _data.frequency = NAN;
  _data.powerFactor = NAN;

  _data.alarm = 0;
  _data.valid = false;
}

uint16_t PZEM004T_V1::calculateCRC(
  const uint8_t* data,
  uint8_t length
) {

  uint16_t crc = 0xFFFF;

  for (uint8_t i = 0; i < length; i++) {

    crc ^= data[i];

    for (uint8_t j = 0; j < 8; j++) {

      if (crc & 0x0001) {
        crc = (crc >> 1) ^ 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }

  return crc;
}

bool PZEM004T_V1::readRegisters(
  uint16_t startRegister,
  uint16_t quantity,
  uint16_t* registers
) {

  uint8_t request[8];

  request[0] = _address;
  request[1] = MODBUS_READ_INPUT;

  request[2] = startRegister >> 8;
  request[3] = startRegister & 0xFF;

  request[4] = quantity >> 8;
  request[5] = quantity & 0xFF;

  uint16_t crc = calculateCRC(request, 6);

  request[6] = crc & 0xFF;
  request[7] = crc >> 8;

  while (_serial->available()) {
    _serial->read();
  }

  _serial->write(request, sizeof(request));

  uint8_t response[25];

  uint8_t count = 0;

  unsigned long startTime = millis();

  while (millis() - startTime < PZEM_TIMEOUT) {

    if (_serial->available()) {

      if (count < sizeof(response)) {
        response[count++] = _serial->read();
      } else {
        _serial->read();
      }

      if (count >= 2 && (response[1] & 0x80)) {
        if (count >= 5) break;
      }

      if (count >= 3 && !(response[1] & 0x80)) {

        uint8_t expectedLength = response[2] + 5;

        if (count >= expectedLength) {
          break;
        }
      }
    }
  }

  if (count == 0) {
    _error = PZEM_ERROR_TIMEOUT;
    return false;
  }

  if (count < 5) {
    _error = PZEM_ERROR_FRAME;
    return false;
  }

  uint16_t receivedCRC =
    response[count - 2] |
    ((uint16_t)response[count - 1] << 8);

  uint16_t calculatedCRC =
    calculateCRC(response, count - 2);

  if (receivedCRC != calculatedCRC) {
    _error = PZEM_ERROR_CRC;
    return false;
  }

  if (response[0] != _address) {
    _error = PZEM_ERROR_ADDRESS;
    return false;
  }

  if (response[1] & 0x80) {
    _error = PZEM_ERROR_EXCEPTION;
    return false;
  }

  if (response[1] != MODBUS_READ_INPUT) {
    _error = PZEM_ERROR_FUNCTION;
    return false;
  }

  if (response[2] != quantity * 2 ||
      count != response[2] + 5) {

    _error = PZEM_ERROR_FRAME;
    return false;
  }

  for (uint8_t i = 0; i < quantity; i++) {

    registers[i] =
      ((uint16_t)response[3 + i * 2] << 8) |
      response[4 + i * 2];
  }

  _error = PZEM_OK;

  return true;
}

bool PZEM004T_V1::read() {

  clearData();

  uint16_t registers[PZEM_REGISTER_COUNT];

  if (!readRegisters(
    PZEM_FIRST_REGISTER,
    PZEM_REGISTER_COUNT,
    registers
  )) {

    return false;
  }

  uint32_t rawCurrent =
    ((uint32_t)registers[2] << 16) |
    registers[1];

  uint32_t rawPower =
    ((uint32_t)registers[4] << 16) |
    registers[3];

  uint32_t rawEnergy =
    ((uint32_t)registers[6] << 16) |
    registers[5];

  _data.voltage =
    registers[0] / 10.0;

  _data.current =
    rawCurrent / 1000.0;

  _data.power =
    rawPower / 10.0;

  _data.energyWh =
    (float)rawEnergy;

  _data.frequency =
    registers[7] / 10.0;

  _data.powerFactor =
    registers[8] / 100.0;

  _data.alarm =
    registers[9];

  _data.valid = true;

  return true;
}

PZEMData PZEM004T_V1::getData() {
  return _data;
}

float PZEM004T_V1::voltage() {
  return _data.voltage;
}

float PZEM004T_V1::current() {
  return _data.current;
}

float PZEM004T_V1::power() {
  return _data.power;
}

float PZEM004T_V1::energyWh() {
  return _data.energyWh;
}

float PZEM004T_V1::energyKWh() {
  return _data.energyWh / 1000.0;
}

float PZEM004T_V1::frequency() {
  return _data.frequency;
}

float PZEM004T_V1::powerFactor() {
  return _data.powerFactor;
}

uint16_t PZEM004T_V1::alarm() {
  return _data.alarm;
}

PZEMError PZEM004T_V1::getError() {
  return _error;
}

const char* PZEM004T_V1::getErrorMessage() {

  switch (_error) {

    case PZEM_OK:
      return "Aucune erreur";

    case PZEM_ERROR_TIMEOUT:
      return "Timeout";

    case PZEM_ERROR_CRC:
      return "Erreur CRC";

    case PZEM_ERROR_ADDRESS:
      return "Adresse incorrecte";

    case PZEM_ERROR_FUNCTION:
      return "Fonction Modbus incorrecte";

    case PZEM_ERROR_FRAME:
      return "Trame incorrecte";

    case PZEM_ERROR_EXCEPTION:
      return "Exception Modbus";

    default:
      return "Erreur inconnue";
  }
}

void PZEM004T_V1::setAddress(uint8_t address) {

  if (address >= 0x01 && address <= 0xF7) {
    _address = address;
  }
}
