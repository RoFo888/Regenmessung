//Generice ESP8266 Module 4M/1M SPIFFS für ESP-07 mit modifiziertem Chip
//Generice ESP8266 Module 4M/1M SPIFFS für ESP-12E
//normales ESP-07 funktioniert nicht mit SPIFFS+EEPROM+OTA
//ESP-07s mit 4MBit und Antenne ohne LED

//Version 0.11                      //erste lauffähige Version
//Version 0.11                      //Verbesserung für Serverlog
//Version 0.12                      //Implementierung RTC Log
//Version 0.13                      //07.08.2018 Implementierung Abfrage Debugmodus
//Version 0.14                      //07.08.2018 Implementierung EEPROM Fail Send und Offline log Send
//Version 0.15                      //Testversion RTC Sync
//Version 0.16                      //30.12.2018 Neuversuch da Speicherprobleme
//Version 0.17                      //01.01.2019 Umsellung auf POST
//Version 0.18                      //06.01.2019 Überprüfung Webtime und RTC Time nach RTC Ausfall
//Version 0.19                      //22.11.2019 neue Arduino IDE und ESP8266 Version
//Version 0.20                      //28.03.2020 neues Board mit INA219 und solar
#define Version 0.21                //20.04.2020 JLCPCB Borad V1 + neue POST Variante

const char* ssid = "FRITZ!Box 7490-1";
const char* password = "07023708331231881462";
//const char* ssid = "WLAN-277264";
//const char* password = "7556988665396178";
const String path = "/test/";
//const String path = "/Regenmessung/";

const char* web = "rofo.lima-city.de";
const String host = "rofo.lima-city.de";




#define Systemname "ESP8266-Regenmessung-Testsystem"   //Name für Host
#define I2C_PowerPin 12                     //Pin für RTC Spannungsversorgung (D8 auf NodeMCE, 12 auf ESP-12)
#define RTC_I2C_ADDRESS 0x68                //I2C Adresse des RTC  DS3231
#define INA219_I2C_ADDRESS 0x40             //I2C Adresse für Strom/Spannungsmessung
#define buffersize 1000                     //Puffergröße für POST Methode
#define ControlRegister 0x0E
#define StatusRegister  0x0F

#include <EEPROM.h>
#include "FS.h"
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <ArduinoOTA.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;
WiFiClientSecure https;

int jahr, monat, tag, stunde, minute, sekunde, wochentag;
int web_jahr, web_monat, web_tag, web_stunde, web_minute, web_sekunde, web_wochentag;
float shuntvoltage = 0, busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;
float power_mW = 0;
bool I2CisOK;

void setup() {
  https.setInsecure();
  Serial.begin(115200);
  Serial.println();
  SPIFFS.begin();
  WriteSPIFFSlog("===> Starte Regenmessung - Version:" + String(Version) + " <===");
  Serial.println("===> Starte Regenmessung - Version:" + String(Version) + " <===");
  Serial.println("Starting EEPROM");
  EEPROM.begin(4);
  Serial.println("Sendfail is : " + String(sendfail()));
  WriteSPIFFSlog("Sendfail is : " + String(sendfail()));
  pinMode(I2C_PowerPin, OUTPUT);        //Spannung für I2C vorbereiten
  digitalWrite(I2C_PowerPin, HIGH);     //I2C Spannung ein
  Wire.begin(14, 13);                   //(sda,scl)for ESP-07S Kommunikation über die Wire.h Bibliothek beginnen für RTC
  TestI2C();                            //alle I2C Teilnehmer vorhanden?
  if (I2CisOK) {                        //Ausrufezeichen entfernen!
    if (readbit(StatusRegister, 7)) Serial.println("RTC may corrupted!!!");
    writeRegentoSPIFFS();               //Schreibe Regenzeit ins SPIFFS
    getVCC_init();
  }
  if (connectWiFi())                              //wenn WiFi OK dann...
  {
    if (debugmode(host, path)) blinkforever();    //DebugMode?
    WritetoWeb(host, path, "regen.php");          //Schreibe Regenzeit ins WEB
    Send_logfile_once_a_month();
    WriteSPIFFSlog("RTC:" + printRTCDateTime() + "   WEB:" + printWEBDateTime());
    Serial.println("RTC:" + printRTCDateTime() + "   WEB:" + printWEBDateTime());
    if (sendfail()) {
      HTTPS_POST2("regen_offline");
      HTTPS_POST2("logfile");
      sendfailstate(false);
    }
  }
  else
  {
    sendfailstate(true);//Senden Fehlgeschlagen
  }
  Sleep();
}

void loop() {
}//nothing to do :)

void Send_logfile_once_a_month() {
  if (monat != ReadMonat()) {
    WriteSPIFFSlog("aktueller Monat in RTC :" + String(monat) + " ungleich Monat in EEPROM : Sende Logfile");
    Serial.println("aktueller Monat in RTC " + String(monat) + " ungleich Monat in EEPROM : Sende Logfile");
    HTTPS_POST2("logfile");
    WriteMonat(monat);
  }
}

void getVCC_init() {
  ina219.begin();
  ina219.setCalibration_16V_400mA();
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  Serial.println("U_Batt : " + String(loadvoltage) + " V");
  WriteSPIFFSlog("U_Batt : " + String(loadvoltage) + " V");
}

byte ReadMonat() {
  Serial.println(EEPROM.read(1));
  return EEPROM.read(1);
}

bool WriteMonat(byte Monat) {
  EEPROM.write(1, Monat);
  EEPROM.commit();
}

bool sendfail()
{
  if (EEPROM.read(0) == 111) return true;
  if (EEPROM.read(0) == 222) return false;
}

void sendfailstate(bool sendstatus) {
  if (sendstatus)
  {
    EEPROM.write(0, 111);
    Serial.println("Sendfail set to TRUE");
  }
  else
  {
    EEPROM.write(0, 222);
    Serial.println("Sendfail set to FALSE");
  }
  EEPROM.commit();
}

String getVCC() {
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  return (String(loadvoltage) + " V");
}

void blinkforever() {
  Serial.println(printRTCDateTime() + " Enter DebugMode : blinking for ever");
  WriteSPIFFSlog(printRTCDateTime() + " Enter DebugMode : blinking for ever");
  PrintCommand();
  ArduinoOTA.setHostname(Systemname);
  ArduinoOTA.begin();
  while (1) {
    ArduinoOTA.handle();
    if (Serial.available()) {
      byte incomingByte = Serial.read();
      if (incomingByte == 'D') DelSPIFFSlogfile();
      if (incomingByte == 'S') HTTPS_POST2("logfile");
      if (incomingByte == 'R') ReadSPIFFSlogfile();
      if (incomingByte == '1') DelSPIFFSRegenlog();
      if (incomingByte == '2') HTTPS_POST2("regen_offline");
      if (incomingByte == '3') ReadSPIFFSRegenlog();
      if (incomingByte == 'V') Serial.println(getVCC());
      if (incomingByte == '~') Sleep();
      if (incomingByte == 'T') Serial.println(printRTCDateTime());
      if (incomingByte == 'W') Serial.println(printWEBDateTime());
      if (incomingByte == 'F') Serial.println(SPIFFS.format());
      if (incomingByte == 'Z') SyncTime();
      if (incomingByte == 'X') ReSet();
      if (incomingByte == '5') WritetoWeb(host, path, "regen.php");
      if (incomingByte == '8')ReadMonat();
    }
  }
}

void PrintCommand() {
  Serial.println("S = POST LogFile");
  Serial.println("2 = POST Regen_OfflineFile");
  Serial.println("1 = Del Regen_OfflineFile SPIFFS");
  Serial.println("D = Del Logfile SPIFFS");
  Serial.println("R = ReadSPIFFSlogfile");
  Serial.println("3 = ReadSPIFFSRegenlog");
  Serial.println("F = Format SPIFFS");
  Serial.println("5 = WritetoWeb");
  Serial.println("V = Read Ubat");;
  Serial.println("~ = goto Sleep");
  Serial.println();
  Serial.println("T = PrintRTCDateTime");
  Serial.println("W = PrintWEBDateTime");
  Serial.println("Z = SyncTime");
  Serial.println("X = ReSet");
  Serial.println("8 = Read Monat EEPROM");

}
void ReSet() {
  ESP.restart();
}

bool debugmode(String _host, String _path) {
  String Antwort = ReadfromWeb(host, path, "config.txt");
  Antwort.remove(0, Antwort.indexOf("DebugMode")); //alles davor Abschneiden
  Antwort.remove(Antwort.indexOf("\r\n"));//alles danach Abschneiden
  Antwort.remove(0, Antwort.indexOf("=") + 1); //alles davor Abschneiden
  Antwort.trim();
  Serial.printf("DEBUGMODE is %s \r\n", Antwort == "1" ? "true" : "false");
  if (Antwort == "1")return true;
  return false;
}

void GetWebTime() {
  String Antwort = ReadfromWeb(host, path, "gettime.php");
  //Serial.println(Antwort);
  Antwort.remove(0, Antwort.indexOf("DEUTSCHE UHRZEIT") + 16); //alles davor Abschneiden
  Antwort.trim();
  Antwort.remove(Antwort.indexOf("\r\n"));//alles danach Abschneiden
  web_sekunde = Antwort.substring(18, 20).toInt(); //Sekunde
  web_minute = Antwort.substring(15, 17).toInt(); //Minute
  web_stunde = Antwort.substring(12, 14).toInt(); //Stunde
  web_tag = Antwort.substring(0, 2).toInt(); //TAG
  web_monat = Antwort.substring(3, 5).toInt(); //MONAT
  web_jahr = Antwort.substring(6, 10).toInt(); //JAHR
}

void SyncTime() {
  GetWebTime();
  Wire.beginTransmission(RTC_I2C_ADDRESS);
  Wire.write(0); // Der Wert 0 aktiviert das RTC Modul.
  Wire.write(decToBcd(web_sekunde)); //Sekunde
  Wire.write(decToBcd(web_minute)); //MINUTE
  Wire.write(decToBcd(web_stunde)); //Stunde
  Wire.write(decToBcd(0)); // Wochentag unberücksichtigt
  Wire.write(decToBcd(web_tag)); //TAG
  Wire.write(decToBcd(web_monat)); //MONAT
  Wire.write(decToBcd(web_jahr - 2000)); //JAHR
  Wire.endTransmission();
  writeregister(StatusRegister, writebit(StatusRegister, 7, false)); //Schreibe OSF to 0
  writeregister(StatusRegister, writebit(StatusRegister, 3, false)); //Schreibe 32kHz
  writeregister(ControlRegister, writebit(ControlRegister, 7, false)); //Schreibe OSF to 0
  Serial.println("RTC set to " + printRTCDateTime());
  Wire.beginTransmission(RTC_I2C_ADDRESS);  // transmit to device
  Wire.write(0x0B);                            // set read pointer to 0x0e
  Wire.endTransmission();    // stop transmitting
  Wire.requestFrom(RTC_I2C_ADDRESS, 7);
  byte A2_1 = Wire.read();
  byte A2_2 = Wire.read();
  byte A2_3 = Wire.read();
  byte Control = Wire.read();
  byte Status = Wire.read();
  Serial.print("Control Register(7 to 0) : ");
  Serial.println();
  prntBits(Control);
  Serial.println("A1IE :" + String(bitRead(Control, 0)));
  Serial.println("A2IE :" + String(bitRead(Control, 1)));
  Serial.println("INTCS :" + String(bitRead(Control, 2)));
  Serial.println("RS1 :" + String(bitRead(Control, 3)));
  Serial.println("RS2 :" + String(bitRead(Control, 4)));
  Serial.println("CONV :" + String(bitRead(Control, 5)));
  Serial.println("BBSQW :" + String(bitRead(Control, 6)));
  Serial.println("EOSC :" + String(bitRead(Control, 7)));
  Serial.println();
  Serial.print("Status Register(7 to 0) : ");
  Serial.println("A1F :" + String(bitRead(Status, 0)));
  Serial.println("A2F :" + String(bitRead(Status, 1)));
  Serial.println("BSY :" + String(bitRead(Status, 2)));
  Serial.println("EN32kHz :" + String(bitRead(Status, 3)));
  Serial.println("OSF :" + String(bitRead(Status, 7)));
  Serial.println();
}

void Sleep() {
  Serial.println(printRTCDateTime() + " Go to DeepSleep!   bye bye");
  WriteSPIFFSlog(printRTCDateTime() + " Go to DeepSleep!   bye bye");
  pinMode(I2C_PowerPin, INPUT);        //RTC Spannung aus
  Serial.flush();
  ESP.deepSleep(0);
  delay(1);
}

void rtcReadTime() {
  Wire.beginTransmission(RTC_I2C_ADDRESS); //Aufbau der Verbindung zur Adresse 0x68
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(RTC_I2C_ADDRESS, 7);
  sekunde    = bcdToDec(Wire.read() & 0x7f);
  minute     = bcdToDec(Wire.read());
  stunde     = bcdToDec(Wire.read() & 0x3f);
  wochentag  = bcdToDec(Wire.read());
  tag        = bcdToDec(Wire.read());
  monat      = bcdToDec(Wire.read());
  jahr       = bcdToDec(Wire.read()) + 2000;
}

//Convertiert Dezimalzeichen in binäre Zeichen.
byte decToBcd(byte val) {
  return ( (val / 10 * 16) + (val % 10) );
}

//Convertiert binäre Zeichen in Dezimal Zeichen.
byte bcdToDec(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}

//Berechnet den Tag der Woche aus dem übergebenen Datumswerten.
byte calcDayOfWeek(int jahr, byte monat, byte tag) {
  static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  jahr -= monat < 3;
  return ((jahr + jahr / 4 - jahr / 100 + jahr / 400 + t[monat - 1] + tag) % 7);
}

String printRTCDateTime() {
  if (I2CisOK) {
    rtcReadTime();
  }
  else {
    return ("no Time available : ");
  }
  if ((tag > 31) || (monat > 12) || (stunde > 24) || (minute > 60)) return ("no Time available : ");
  String result = "";
  if (tag < 10) result += "0";
  result += tag;
  result += ".";
  if (monat < 10) result += "0";
  result += monat;
  result += ".";
  result += jahr;
  result += " ";
  if (stunde < 10) result += "0";
  result += stunde;
  result += ":";
  if (minute < 10) result += "0";
  result += minute;
  result += ":";
  if (sekunde < 10) result += "0";
  result += sekunde;
  return (result);
}

String printWEBDateTime() {
  GetWebTime();
  String result = "";
  if (web_tag < 10) result += "0";
  result += web_tag;
  result += ".";
  if (web_monat < 10) result += "0";
  result += web_monat;
  result += ".";
  result += web_jahr;
  result += " ";
  if (web_stunde < 10) result += "0";
  result += web_stunde;
  result += ":";
  if (web_minute < 10) result += "0";
  result += web_minute;
  result += ":";
  if (web_sekunde < 10) result += "0";
  result += web_sekunde;
  return (result);
}
