void HTTPS_POST(String logfile) {
  File myfile = SPIFFS.open("/" + logfile + ".txt", "r");
  Serial.println ("connecting to https://" + host + " to send " + logfile + ".txt via POST");
  if (!https.connect(web, 443)) {
    Serial.println("connection failed");
    return;
  }
  String url = path + "post" + logfile + ".php"; // We now create a URI for the request
  Serial.print("Requesting URL: ");
  Serial.println(String("https://") + host + url + " to send " + String(myfile.size()) + " Bytes payload");
  // This will send the request to the server
  https.println("POST /" + path + "/post" + logfile + ".php HTTP/1.1");
  https.println("Host: " + host);
  https.println("Cache-Control: no-cache");
  https.println("Content-Type: application/x-www-form-urlencoded");
  https.print("Content-Length: ");
  https.println(myfile.size());  //Länge
  https.println();               //Letzte Zeile, danach Payload
  while (myfile.position() < myfile.size()) {
    String s = myfile.readStringUntil('\r');
    s.trim();
    if (s.length() != 0) https.println(s); //ab hier payload
  }
  // Read all the lines of the reply from server and print them to Serial
  String Antwort;
  delay(1500);
  while (https.available()) {
    char c = https.read();
    Serial.print(c);
    //Antwort += c;
  }
  Serial.println();
  Serial.println("closing connection");
  https.stop();
  myfile.close();
}
