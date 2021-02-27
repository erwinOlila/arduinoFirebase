#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <stdio.h>
#include <stdlib.h>
#include <RTClib.h>

WiFiMulti wifiMulti;
HTTPClient http;

const char* ssid = "*******";     // type your wifi ssid
const char* pass = "*******";    // type your wifi password

const int connected_led= 23;
const int sound_sensor = 36;
const int success_send = 15;
const int fail_send = 2;

RTC_DS3231 rtc;

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(connected_led, OUTPUT);
  pinMode(success_send, OUTPUT);
  pinMode(fail_send, OUTPUT);
  pinMode(sound_sensor, INPUT);
  
  digitalWrite(connected_led, LOW);
  digitalWrite(success_send, LOW);

  if(!rtc.begin()) {
    while(1);
  }
//  rtc.adjust(DateTime(__DATE__, __TIME__)); 

  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(connected_led, HIGH);

  String message = fireGetData();
  Serial.println(message);
  firePutData();
}

void loop() {

  if(WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, pass);
    while(WiFi.status() != WL_CONNECTED) {
      digitalWrite(connected_led, LOW);
      digitalWrite(fail_send, HIGH);
      delay(500);
      Serial.print(".");
    }
  }
  digitalWrite(connected_led, HIGH);
  digitalWrite(fail_send, LOW);

  firePutData();
  delay(300000);

}

uint32_t getTime(){
  DateTime now = rtc.now();
  return now.unixtime();
}

char* toString(int number) {
  const int len = snprintf(NULL, 0, "%d", number);
  char* snum = (char*)malloc(len + 1);
  snprintf(snum, len + 1, "%lu", number);

  return snum;
}

String fireGetData() {
  
  const char* url = "https://home-temperature-data-1ef85-default-rtdb.firebaseio.com/test.json";
  http.setTimeout(1000);
  http.begin(url);
  int httpCode = http.GET();
  String payload;

  if(httpCode > 0) {
    Serial.println("[HTTP] GET... code: ");
    Serial.println(httpCode);

    if(httpCode == HTTP_CODE_OK) {
      payload = http.getString();
      return payload;
    }
  } else {
    Serial.println("[HTTP] GET... failed, error:");
    Serial.println(http.errorToString(httpCode));
  }
  http.end();
}

void firePutData() {

  char* key = toString(getTime());

  char* str1 = "https://home-temperature-data-1ef85-default-rtdb.firebaseio.com/";
  char* str2 = key;
  char combine[150] = "";

  sprintf(combine, "%s%s.json", str1, str2);
  const char* url = combine;
  Serial.println(url);
  
  http.setTimeout(1000);
  http.begin(url);
  http.addHeader("Content-Type", "text/plain");

  const int sound = analogRead(sound_sensor);

  Serial.println(key);
  Serial.println(sound);
  
  char* value = toString(sound);
  
  int httpCode = http.PUT(value);
  
  String payload;

  if(httpCode > 0) {
    Serial.println("[HTTP] GET... code: ");
    Serial.println(httpCode);

    if(httpCode == HTTP_CODE_OK) {
      payload = http.getString();
      digitalWrite(success_send, HIGH);
      delay(100);
      digitalWrite(success_send, LOW);
      delay(100);
      digitalWrite(success_send, HIGH);
      delay(100);
      digitalWrite(success_send, LOW);
    }
  } else {
    Serial.println("[HTTP] GET... failed, error:");
    Serial.println(http.errorToString(httpCode));
    digitalWrite(fail_send, HIGH);
    delay(100);
    digitalWrite(fail_send, LOW);
    delay(100);
    digitalWrite(fail_send, HIGH);
    delay(100);
    digitalWrite(fail_send, LOW);
  }
  http.end();
}
