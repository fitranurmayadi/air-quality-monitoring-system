#include "Communication.h"
#include "LocalServer.h"
#include "OLED.h"
#include "RTC.h"
#include "Sensors.h"
#include "Storage.h"

void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(ledDust, OUTPUT);
  pinMode(buzz, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  }
  if (!bme.begin(0x76)) {
    Serial.println("bme280 gagal diinisiasi");
  }
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  display_one_time(0, 32, "Loading.....");
  delay(1000);

  display_ap_notification();
  //setup wifimanager
  setupWM();

  Serial.println("connected...yeey :)");
  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.on("/reset", handle_reset);
  server.onNotFound(handle_NotFound);
  server.begin();
}


void updateScreen() {
  if (displayScreenNum == 0) {
    String timeNow = getTimeRTC();
    String dateNow = getDateRTC();
    displayLocalTime(timeNow, dateNow);
  }
  else if (displayScreenNum == 1) {

    float temp = NAN, hum = NAN, pres = NAN;
    if (bme280_read(&temp, &hum, &pres)) {
      displayTempNHum(String(temp), String(hum) );
    }
  }
  else if (displayScreenNum == 2) {
    uint16_t no2 = 0U, nh3 = 0U, co = 0U;
    if (mics6814_read(&no2, &nh3, &co)) {
      displaySensor1(String(co), String(nh3), String(no2));
    }
  }
  else if (displayScreenNum == 3) {
    float so2 = so2_read();
    float h2s = mq136_read();
    float dust = dust_read();
    displaySensor2(String(so2), String(h2s), String(dust));
  }
  else {
    displayNetwork(statusNetwork);
  }
}

void sensor_task() {
  String httpRequestData = "api_key=" + String(api_token);

  String th = "", mic = "", sulf = "", debu = "";
  float temp = NAN, hum = NAN, pres = NAN;
  uint16_t no2 = 0U, nh3 = 0U, co = 0U;

  float so2 = so2_read();
  float h2s = mq136_read();
  float dust = dust_read();

  if (bme280_read(&temp, &hum, &pres)) {
    th = String(temp) + " C  " + String(hum) + " %";
    httpRequestData += "&field1=" + String(temp) + "&field2=" + String(hum);
  }

  if (mics6814_read(&no2, &nh3, &co)) {
    mic = "NO2: " + String(no2) + ", NH3: " + String(nh3) + ", CO: " + String(co);
    httpRequestData += "&field3=" + String(no2) + "&field4=" + String(nh3) + "&field5=" + String(co);
  }

  sulf = "SO2: " + String(so2) + ", H2S: " + String(h2s);
  debu = "Dust: " + String(dust);
  httpRequestData += "&field6=" + String(so2) + "&field7=" + String(h2s) + "&field8=" + String(dust);


  if ((millis() - lastTime) > timerDelay) { //send data to thingspeak every 5 minutes
    connectionCheck();
    if (WiFi.status() == WL_CONNECTED) {
      send_to_thingspeak(httpRequestData);
      digitalWrite(buzz, HIGH);
      delay(100);
      digitalWrite(buzz, LOW);
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }

  if ((millis() - lastTimerDisplay) > timerDelayDisplay) {
    updateScreen();
    //Serial.println(displayScreenNum);
    if (displayScreenNum < displayScreenNumMax) {
      displayScreenNum++;
    }
    else {
      displayScreenNum = 0;
    }
    lastTimerDisplay = millis();
  }
}

void loop() {
  server.handleClient();
  sensor_task();
}
