#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// Screens
int displayScreenNum = 0;
int displayScreenNumMax = 4;

unsigned long lastTimerDisplay = 0;
unsigned long timerDelayDisplay = 10000;


unsigned char temperature_icon[] = {
  0b00000001, 0b11000000, //        ###
  0b00000011, 0b11100000, //       #####
  0b00000111, 0b00100000, //      ###  #
  0b00000111, 0b11100000, //      ######
  0b00000111, 0b00100000, //      ###  #
  0b00000111, 0b11100000, //      ######
  0b00000111, 0b00100000, //      ###  #
  0b00000111, 0b11100000, //      ######
  0b00000111, 0b00100000, //      ###  #
  0b00001111, 0b11110000, //     ########
  0b00011111, 0b11111000, //    ##########
  0b00011111, 0b11111000, //    ##########
  0b00011111, 0b11111000, //    ##########
  0b00011111, 0b11111000, //    ##########
  0b00001111, 0b11110000, //     ########
  0b00000111, 0b11100000, //      ######
};

unsigned char humidity_icon[] = {
  0b00000000, 0b00000000, //
  0b00000001, 0b10000000, //        ##
  0b00000011, 0b11000000, //       ####
  0b00000111, 0b11100000, //      ######
  0b00001111, 0b11110000, //     ########
  0b00001111, 0b11110000, //     ########
  0b00011111, 0b11111000, //    ##########
  0b00011111, 0b11011000, //    ####### ##
  0b00111111, 0b10011100, //   #######  ###
  0b00111111, 0b10011100, //   #######  ###
  0b00111111, 0b00011100, //   ######   ###
  0b00011110, 0b00111000, //    ####   ###
  0b00011111, 0b11111000, //    ##########
  0b00001111, 0b11110000, //     ########
  0b00000011, 0b11000000, //       ####
  0b00000000, 0b00000000, //
};

unsigned char wifi_icon[] = {
  0b00000011, 0b11000000,
  0b00001111, 0b11110000,
  0b00111100, 0b00111100,
  0b01110000, 0b00001110,
  0b11100001, 0b10000111,
  0b11000111, 0b11100011,
  0b10011111, 0b11111001,
  0b00111100, 0b00111100,
  0b00110000, 0b00001100,
  0b00100001, 0b10000100,
  0b00000111, 0b11100000,
  0b00001110, 0b01110000,
  0b00001100, 0b00110000,
  0b00000000, 0b00000000,
  0b00000001, 0b10000000,
  0b00000001, 0b10000000
};


//need to display:
//1. date & time
//2. temperature and humidity
//3. environment sensor
//4.
//5. WiFi and IP



// Create display marker for each screen
void displayIndicator(int displayNumber) {
  int xCoordinates[5] = {44, 54, 64, 74, 84};
  for (int i = 0; i < 5; i++) {
    if (i == displayNumber) {
      display.fillCircle(xCoordinates[i], 60, 2, WHITE);
    }
    else {
      display.drawCircle(xCoordinates[i], 60, 2, WHITE);
    }
  }
}

//SCREEN NUMBER 0: DATE AND TIME
void displayLocalTime(String timeNow, String dateNow) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(3);
  display.setCursor(19, 5);
  display.print(timeNow);
  display.setTextSize(1);
  display.setCursor(16, 40);
  display.print(dateNow);
  displayIndicator(displayScreenNum);
  display.display();
}
//SCREEN NUMBER 1: TEMP. and HUMIDITY
void displayTempNHum(String temperature, String humidity) {
  display.clearDisplay();
  display.setTextSize(2);
  display.drawBitmap(15, 5, temperature_icon, 16, 16 , 1);
  display.setCursor(35, 5);
  display.print(temperature);
  display.cp437(true);
  display.setTextSize(1);
  display.print(" ");
  display.write(167);
  display.print("C");
  
  display.setTextSize(2);
  display.drawBitmap(15, 35, humidity_icon, 16, 16 , 1);
  display.setCursor(35, 35);
  display.print(humidity);
  display.print(" %");
  displayIndicator(displayScreenNum);
  display.display();
}

//SCREEN NUMBER 2: CarbonDioxide, Ammonia, & NitrogenDioxide
void displaySensor1(String co, String nh3, String no2) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print("CO : ");
  display.println(co);
  display.setCursor(0,20);
  display.print("NH3: ");
  display.println(nh3);
  display.setCursor(0,40);
  display.print("NO2: ");
  display.println(no2);
  displayIndicator(displayScreenNum);
  display.display();
}

//SCREEN NUMBER 3: SulfurDioxide, HidrogenSulfide, & Dust
void displaySensor2(String so2, String h2s, String dust) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print("SO2: ");
  display.println(so2);
  display.setCursor(0,20);
  display.print("H2S: ");
  display.println(h2s);
  display.setCursor(0,40);
  display.print("DUST: ");
  display.println(dust);
  displayIndicator(displayScreenNum);
  display.display();
}

//SCREEN NUMBER 4: Network
void displayNetwork(String statusNetwork) {
  display.clearDisplay();
  display.setTextSize(1);
  display.drawBitmap(0, 5, wifi_icon, 16, 16 , 1);
  display.setCursor(35, 5);
  display.println(WiFi.localIP());
  display.setTextSize(1.5);
  display.setCursor(0,25);
  display.print("Status Data:");
  display.print(statusNetwork);
  displayIndicator(displayScreenNum);
  display.display();
}

void display_one_time(int x, int y , String data) {
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x, y);
  display.println(data);
  display.display();
}

void display_line(int x, int y , String data) {
  display.setCursor(x, y);
  display.println(data);
  display.display();
}

void display_ap_notification() {
  display.clearDisplay();
  display.setTextSize(2);
  display.drawBitmap(0, 5, wifi_icon, 16, 16 , 1);
  display.setCursor(25, 5);
  display.print("AP MODE");
  display.setTextSize(1.5);
  display.setCursor(0,25);
  display.println("SSID:Air_Monitor");
  display.setCursor(0,35);
  display.println("PASS:green2020");
  display.setCursor(0,45);
  display.println("http://10.0.1.1/");
  display.display();
  delay(100);
}
