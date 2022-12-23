#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

static Adafruit_BME280 bme;

unsigned long lastTime = 0;
unsigned long timerDelay = 300000; // 5minutes: timerdelay/60000 microseconds

#define ledDust 26 //pin led sensor dust gp2y1014au0f
#define adcDust 39 //pin analog sensor dust
#define so2Pin 36 //pin analog sensor ze03

#define h2spin 33
#define no2pin 32
#define nh3pin 35
#define copin 34

#define buzz 25 //pin for buzzer


float so2_read() {
  float voutx, vso2;
  int vadc = analogRead(so2Pin);
  voutx = (float(vadc) / 4095) * 3.3;
  vso2 = (20 / 2.4) * (voutx - 0.6);
  //Serial.println(vadc);
  //Serial.println(voutx);
  //Serial.println(vso2);
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

  //Serial.println(voMeasured);
  //Serial.println(calcVoltage);
  //Serial.println(dustDensity);

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
    analog_value = analog_value + analogRead(h2spin); //add the values for 200
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
  uint16_t tempNo2 = analogRead(no2pin);
  delay(100);
  uint16_t tempNh3 = analogRead(nh3pin);
  delay(100);
  uint16_t tempCo = analogRead(copin);
  delay(100);
  *no2 = tempNo2;
  *nh3 = tempNh3;
  *co = tempCo;
  return result;
}
/*************************************************************************************************************************************************************************88*/
