#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "your ssid";
const char* password = "your passwaord";

// Domain Name with full URL Path for HTTP POST Request
const char* serverName = "http://api.thingspeak.com/update";
// Service API Key
String apiKey = "--------------";


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

static Adafruit_BME280 bme;


#define ledDust 25
#define adcDust 39
#define so2Pin 36

unsigned long lastTime = 0;
// Set timer to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Timer set to 10 seconds (10000)
unsigned long timerDelay = 300000; // 5minutes


/**********************************************************************************************************************************************************************************/
void setup() {
  Serial.begin(115200);
  pinMode(ledDust, OUTPUT);
  WiFi.begin(ssid, password);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  }
  displaying(0, 32, "Loading.....");
  delay(1000);
  if (!bme.begin(0x76)) {
    Serial.println("bme280 gagal diinisiasi");
  }
}

void displaying(int x, int y , String datanya) {
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x, y);
  display.println(datanya);
  display.display();
}
void connectionCheck() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

float so2_read() {
  float voutx, vso2;
  int vadc = analogRead(so2Pin);
  voutx = (float(vadc) / 4095) * 3.3;
  vso2 = (20 / 2.4) * (voutx - 0.6);
  Serial.println(vadc);
  Serial.println(voutx);
  Serial.println(vso2);
  return vso2;
}

float dust_read() {
  unsigned int samplingTime = 280;
  unsigned int deltaTime = 280;
  unsigned int sleepTime = 280;

  float voMeasured = 0;
  float calcVoltage = 0;
  float dustDensity = 0;

  digitalWrite(ledDust, LOW);
  delayMicroseconds(samplingTime);
  voMeasured = analogRead(adcDust);

  delayMicroseconds(deltaTime);
  digitalWrite(ledDust, HIGH);

  calcVoltage = voMeasured * (3.3 / 4095);
  dustDensity = 170 * calcVoltage - 0.1;

  Serial.println(voMeasured);
  Serial.println(calcVoltage);
  Serial.println(dustDensity);
  
  if (dustDensity < 0) {
    dustDensity = 0.00;
  }
  return dustDensity;
}

/**********************************************************************************************************************************************************************************/
float mq136_read() {

  int RL = 47;
  float m = -0.263; //Enter calculated Slope
  float b = 0.42; //Enter calculated intercept
  int Ro = 20; //Enter found Ro value

  float analog_value;

  float VRL; //Voltage drop across the MQ sensor
  float Rs; //Sensor resistance at gas concentration
  float ratio; //Define variable for ratio

  for (int test_cycle = 1 ; test_cycle <= 500 ; test_cycle++) //Read the analog output of the sensor for 200 times
  {
    analog_value = analog_value + analogRead(33); //add the values for 200
  }

  analog_value = analog_value / 500.0; //Take average
  VRL = analog_value * (3.3 / 4095.0); //Convert analog value to voltage
  //RS = ((Vc/VRL)-1)*RL is the formulae we obtained from datasheet
  Rs = ((3.3  / VRL) - 1) * RL;
  //RS/RO is 3.6 as we obtained from graph of datasheet
  //Ro = Rs / 3.6;
  ratio = Rs / Ro; // find ratio Rs/Ro
  float ppm = pow(10, ((log10(ratio) - b) / m)); //use formula to calculate ppm


  return ppm;
}

/**********************************************************************************************************************************************************************************/
bool bme280_read(float *temp, float *hum, float *pres) {
  bool result = true;
  if ((NULL == temp) || (NULL == hum) || (NULL == pres)) {
    result = false;
  }

  if (true == result) {
    *temp = bme.readTemperature();
    *hum = bme.readHumidity();
    *pres = bme.readPressure() / 100.0F;

    if (isnan(*hum) || isnan(*temp)) {
      result = false;
    }
  }
  return result;
}

/**********************************************************************************************************************************************************************************/
bool mics6814_read(uint16_t* no2, uint16_t* nh3, uint16_t* co) {
  bool result = true;
  uint16_t tempNo2 = analogRead(32);
  delay(100);
  uint16_t tempNh3 = analogRead(35);
  delay(100);
  uint16_t tempCo = analogRead(34);
  delay(100);
  *no2 = tempNo2;
  *nh3 = tempNh3;
  *co = tempCo;

  return result;
}

/**********************************************************************************************************************************************************************************/
void loop() {
  String httpRequestData = "api_key=" + apiKey;

  String th = "";
  String mic = "";
  String sulf = "";
  String debu = "";

  float temp = NAN;
  float hum = NAN;
  float pres = NAN;

  uint16_t no2 = 0U;
  uint16_t nh3 = 0U;
  uint16_t co = 0U;

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

  display.clearDisplay();
  display.setTextSize(1.5); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Air Quality ");
  display.setCursor(0, 18);
  display.println(th);
  display.setCursor(0, 27);
  display.println(mic);
  display.setCursor(0, 36);
  display.println(sulf);
  display.setCursor(0, 57);
  display.println(debu);
  display.display();
  Serial.println(httpRequestData);
  if ((millis() - lastTime) > timerDelay) {
    connectionCheck();
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      int httpResponseCode = http.POST(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
  delay(10000);
}
/**********************************************************************************************************************************************************************************/
