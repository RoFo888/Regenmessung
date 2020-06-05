String ReadfromWeb(String _host, String _path, String data) {
  Serial.println("connecting to https://" + _host + _path +  data);
  if (!https.connect(_host, 443)) {
    Serial.println("connection failed");
    return ("");
  }
  // This will send the request to the server
  https.print(String("GET ") + _path + data + " HTTP/1.1\r\n" +
              "Host: " + _host + "\r\n" +
              "Connection: close\r\n\r\n");
  //Serial.println("Werte an WebServer uebergeben");
  // Read all the lines of the reply from server and print them to Serial
  String Antwort;
  while (https.connected() || https.available()) {
    yield();
    if (https.available())
    {
      yield();
      char c = https.read();
      Antwort += c;
    }
  }
  //Serial.println(Antwort);
  //Serial.println("closing connection");
  //Serial.println();
  https.stop();
  return (Antwort);
}
