#ifndef PZEM004T_V1_H
#define PZEM004T_V1_H

#include <Arduino.h>
#include <SoftwareSerial.h>

struct PZEMData {
  float voltage;
  float current;
  float power;
  float energyWh;
  float frequency;
  float powerFactor;
  uint16_t alarm;

  bool valid;
};

enum PZEMError {
  PZEM_OK = 0,
  PZEM_ERROR_TIMEOUT,
  PZEM_ERROR_CRC,
  PZEM_ERROR_ADDRESS,
  PZEM_ERROR_FUNCTION,
  PZEM_ERROR_FRAME,
  PZEM_ERROR_EXCEPTION
};

class PZEM004T_V1 {

public:

  PZEM004T_V1(
    SoftwareSerial &serial,
    uint8_t address = 0x01
  );

  void begin(uint32_t baudrate = 9600);

  bool read();

  // Remet a zero l'energie cumulee dans le PZEM (operation destructive).
  // Retourne true seulement apres l'accuse de reception Modbus valide.
  bool resetEnergy();

  PZEMData getData();

  float voltage();
  float current();
  float power();
  float energyWh();
  float energyKWh();
  float frequency();
  float powerFactor();

  uint16_t alarm();

  PZEMError getError();

  const char* getErrorMessage();

  void setAddress(uint8_t address);

private:

  SoftwareSerial* _serial;

  uint8_t _address;

  PZEMData _data;

  PZEMError _error;

  uint16_t calculateCRC(
    const uint8_t* data,
    uint8_t length
  );

  bool readRegisters(
    uint16_t startRegister,
    uint16_t quantity,
    uint16_t* registers
  );

  void clearData();

};

#endif
