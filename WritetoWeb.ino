void WritetoWeb(String _host, String _path, String data) {
  Serial.println("connecting to https://" + _host + _path + data);
  if (!https.connect(web, 443)) {
    Serial.println("connection failed");
    return;
  }
  // This will send the request to the server
  https.print(String("GET ") + _path + data + " HTTP/1.1\r\n" +
              "Host: " + _host + "\r\n" +
              "Connection: close\r\n\r\n");
  Serial.println("Werte an WebServer uebergeben");
  delay(1000);
  // Read all the lines of the reply from server and print them to Serial
  String Antwort;
  while (https.available()) {
    char c = https.read();
    //Serial.print(c);
    //Antwort += c;
  }
  //Serial.println(Antwort);
  //Serial.println();
  //Serial.println("closing connection");
  https.stop();
}
