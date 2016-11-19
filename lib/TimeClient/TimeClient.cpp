#include "TimeClient.h"

TimeClient::TimeClient(float utcOffset) {
  myUtcOffset = utcOffset;
}

void TimeClient::updateTime() {
  WiFiClient client;
  const int httpPort = 80;
//  if (!client.connect("www.google.com", httpPort)) {
  if (!client.connect("www.timeapi.org", httpPort)) {
    Serial.println("connection failed");
    return;
  }
//  http://www.timeapi.org/cet/now?\Y-\m-\d%20\H:\M:\S
  // This will send the request to the server
//  client.print(String("GET / HTTP/1.1\r\n") +
//               String("Host: www.google.com\r\n") +
//               String("Connection: close\r\n\r\n"));
  client.print(String("GET /cet/now?%25H:%25M:%25S HTTP/1.1\r\n") +
               String("Host: www.timeapi.org\r\n") +
               String("User-agent:esp8266\r\n") +
               String("Connection: close\r\n\r\n"));
  int repeatCounter = 0;
  while(!client.available() && repeatCounter < 10) {
    delay(1000);
    Serial.println(".");
    repeatCounter++;
  }

  String line;

  int size = 0;
  client.setNoDelay(false);
  while(client.connected()) {
    while((size = client.available()) > 0) {
      line = client.readStringUntil('\n');
      Serial.println(line);
      int parsedHours = line.substring(0, 2).toInt();
      int parsedMinutes = line.substring(3, 5).toInt();
      int parsedSeconds = line.substring(6, 8).toInt();
      Serial.println(String(parsedHours) + ":" + String(parsedMinutes) + ":" + String(parsedSeconds));

      localEpoc = (parsedHours * 60 * 60 + parsedMinutes * 60 + parsedSeconds);
      Serial.println(localEpoc);
      localMillisAtUpdate = millis();

//      line.toUpperCase();
      // example:
      // date: Thu, 19 Nov 2015 20:25:40 GMT
//      if (line.startsWith("DATE: ")) {
//        Serial.println(line.substring(23, 25) + ":" + line.substring(26, 28) + ":" +line.substring(29, 31));
//        int parsedHours = line.substring(23, 25).toInt();
//        int parsedMinutes = line.substring(26, 28).toInt();
//        int parsedSeconds = line.substring(29, 31).toInt();
//        Serial.println(String(parsedHours) + ":" + String(parsedMinutes) + ":" + String(parsedSeconds));
//
//        localEpoc = (parsedHours * 60 * 60 + parsedMinutes * 60 + parsedSeconds);
//        Serial.println(localEpoc);
//        localMillisAtUpdate = millis();
//      }
    }
  }

}

String TimeClient::getHours() {
    if (localEpoc == 0) {
      return "--";
    }
    int hours = ((getCurrentEpochWithUtcOffset()  % 86400L) / 3600) % 24;
    if (hours < 10) {
      return "0" + String(hours);
    }
    return String(hours); // print the hour (86400 equals secs per day)

}
String TimeClient::getMinutes() {
    if (localEpoc == 0) {
      return "--";
    }
    int minutes = ((getCurrentEpochWithUtcOffset() % 3600) / 60);
    if (minutes < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      return "0" + String(minutes);
    }
    return String(minutes);
}
String TimeClient::getSeconds() {
    if (localEpoc == 0) {
      return "--";
    }
    int seconds = getCurrentEpochWithUtcOffset() % 60;
    if ( seconds < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      return "0" + String(seconds);
    }
    return String(seconds);
}

String TimeClient::getFormattedTime() {
  return getHours() + ":" + getMinutes() + ":" + getSeconds();
}

long TimeClient::getCurrentEpoch() {
  return localEpoc + ((millis() - localMillisAtUpdate) / 1000);
}

long TimeClient::getCurrentEpochWithUtcOffset() {
  return round(getCurrentEpoch() + 3600 * myUtcOffset + 86400L) % 86400L;
}
