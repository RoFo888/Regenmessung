void HTTPS_POST2(String logfile) {
  WriteSPIFFSlog("POST2 sendet : " + logfile + ".txt");
  Serial.println("POST2 sendet : " + logfile + ".txt");
  File myfile = SPIFFS.open("/" + logfile + ".txt", "r");
  Serial.println ("connecting to https://" + host + " to send " + logfile + ".txt via POST");
  if (!https.connect(web, 443)) {
    Serial.println("connection failed");
    return;
  }
  Serial.print("Connected: ");
  String url = path + "post" + logfile + ".php"; // We now create a URI for the request
  Serial.print("Requesting URL: ");
  Serial.println(String("https://") + host + url + " to send " + String(myfile.size()) + " Bytes payload");
  // This will send the request to the server
  https.println("POST /" + path + "/post" + logfile + ".php HTTP/1.1");
  https.println("Host: " + host);
  https.println("Cache-Control: no-cache");
  https.println("Content-Type: application/x-www-form-urlencoded");
  https.print  ("Content-Length: ");
  https.println(myfile.size());  //Länge
  https.println();               //Letzte Zeile, danach Payload

  long Startmillis = millis();
  char s[buffersize]; //Buffer anlegen
  while (myfile.size() - myfile.position() > buffersize) {
    myfile.readBytes(s, buffersize);
    s[buffersize] = '\0';
    //Serial.print(s);
    https.print(s);
  }
  int ende = myfile.size() - myfile.position();
  myfile.readBytes(s, myfile.size() - myfile.position());
  s[ende] = '\0';
  //Serial.print(s);
  https.print(s);
  Serial.println("<<<<<END Time : " + String(millis() - Startmillis));
  // Read all the lines of the reply from server and print them to Serial
  String Antwort;
  delay(500);
  while (https.available()) {
    char c = https.read();
    //Serial.print(c);
    Antwort += c;
  }
  Serial.println();
  Serial.println("closing connection");
  https.stop();
  myfile.close();
  if (Antwort.indexOf("Post successfully") == -1) Serial.println("POST2 nicht Erfolgreich!!!  :(");
  if (Antwort.indexOf("Post successfully") == -1) WriteSPIFFSlog("POST2 nicht Erfolgreich!!!  :(");
  if (Antwort.indexOf("Post successfully") != -1) Serial.println("POST2 Erfolgreich gesendet : " + logfile + ".txt in " +  String(millis() - Startmillis) + " ms");
  if (Antwort.indexOf("Post successfully") != -1) WriteSPIFFSlog("POST2 Erfolgreich gesendet : " + logfile + ".txt in " +  String(millis() - Startmillis) + " ms");
  Serial.println("POST2 Ende");

}
