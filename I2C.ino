

void TestI2C() {
  if (CheckI2C(INA219_I2C_ADDRESS) & CheckI2C(RTC_I2C_ADDRESS)) {
    Serial.println("I2C Devices OK");
    WriteSPIFFSlog("I2C Devices OK");
    I2CisOK = true;
  }
  else {
    I2CisOK = false;
  }
}

bool CheckI2C(byte I2CAddress) {
  Wire.beginTransmission(I2CAddress); //Aufbau der Verbindung zur Adresse 0x68
  byte error = Wire.endTransmission();
  if (error == 0) {
    return true;
  }
  else
  { WriteSPIFFSlog("I2C Störung bei Adresse " + String(I2CAddress, HEX));
    Serial.println("I2C Störung bei Adresse " + String(I2CAddress, HEX));
    return false;
  }
}

bool readbit(byte Reg, byte Bit)
{
  return bitRead(readregister(Reg), Bit);
}

byte readregister(byte reg) {
  Wire.beginTransmission(RTC_I2C_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(RTC_I2C_ADDRESS, 1);
  byte i = Wire.read();
  Wire.endTransmission();
  return i;
}

byte writeregister(byte reg, byte data) {
  Wire.beginTransmission(RTC_I2C_ADDRESS); //Aufbau der Verbindung zur Adresse 0x68
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

byte writebit(byte Reg, byte Bit, bool value) {
  byte i = readregister(Reg);
  bitWrite(i, Bit, value);
  return i;
}

void prntBits(byte b)
{
  for (int i = 7; i >= 0; i--)
    Serial.print(bitRead(b, i));
  Serial.println();
}
