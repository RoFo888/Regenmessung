bool connectWiFi()
{ WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int temp = 0;
  while ((WiFi.status() != WL_CONNECTED) && (temp < 45)) {
    delay(500);
    temp++;
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    WriteSPIFFSlog(printRTCDateTime() + " WiFi connected to " + WiFi.localIP().toString() + " mit RIIS:" +  WiFi.RSSI());
    Serial.println(printRTCDateTime() + " WiFi connected to " + WiFi.localIP().toString() + " mit RIIS:" +  WiFi.RSSI());
    return true;
  }
  else {
    WriteSPIFFSlog(printRTCDateTime() + " WiFi NOT connected");
    Serial.println(printRTCDateTime() + " WiFi NOT connected");
    return false;
  }
}
