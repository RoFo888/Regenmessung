void WriteSPIFFSlog(String logging)
{ File mylogfile = SPIFFS.open("/logfile.txt", "a");
  if (!mylogfile) {
    Serial.println("file open failed");
  }
  mylogfile.println(logging);
  mylogfile.close();
}

void writeRegentoSPIFFS() {
  WriteSPIFFSlog(printRTCDateTime() + " Schreibe in SPIFFS regen_offline.txt");
  Serial.println(printRTCDateTime() + "  Schreibe in SPIFFS regen_offline.txt");
  { File mylogfile = SPIFFS.open("/regen_offline.txt", "a");
    if (!mylogfile) {
      Serial.println("file open failed");
      return;
    }
    Serial.println("Schreibe : " + printRTCDateTime() + " to SPIFFS regen_offline.txt");
    mylogfile.println(printRTCDateTime());
    mylogfile.flush();
    mylogfile.close();
  }
}

void ReadSPIFFSRegenlog()
{ File mylogfile = SPIFFS.open("/regen_offline.txt", "r");
  Serial.print("Lese komplettes regen_offline.txt LogFile mit Byte :");
  Serial.println(mylogfile.size());
  Serial.println();
  while (mylogfile.position() < mylogfile.size()) {
    String s = mylogfile.readStringUntil('\n');
    s.trim();
    Serial.println(s);
  }
  mylogfile.close();
  mylogfile.flush();
  Serial.println();
  Serial.println("komplettes regen_offline.txt LogFile gelesen");
}

void ReadSPIFFSlogfile()
{ File mylogfile = SPIFFS.open("/logfile.txt", "r");
  Serial.print("Lese komplettes logfile.txt LogFile mit Byte");
  Serial.println(mylogfile.size());
  Serial.println();
  while (mylogfile.position() < mylogfile.size()) {
    String s = mylogfile.readStringUntil('\n');
    s.trim();
    Serial.println(s);
  }
  mylogfile.close();
  Serial.println();
  Serial.println("komplettes logfile.txt LogFile gelesen");
}

void DelSPIFFSlogfile()
{ Serial.println("starte Loeschung von logfile.txt");
  if (SPIFFS.remove("/logfile.txt")) {
    Serial.println("Logfile geloescht");
  }
  else
  {
    Serial.println("Logfile loeschen nicht moeglich!");
  }
  Serial.println("Loeschung von logfile.txt abgeschlossen");
}

void DelSPIFFSRegenlog()
{ Serial.println("starte Loeschung von regen_offline.txt");
  if (SPIFFS.remove("/regen_offline.txt")) {
    Serial.println("Logfile geloescht");
  }
  else
  {
    Serial.println("Logfile loeschen nicht moeglich!");
  }
  Serial.println("Loeschung von regen_offline.txt abgeschlossen");
}
