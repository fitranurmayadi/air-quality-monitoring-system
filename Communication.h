#include <FS.h>          // this needs to be first, or it all crashes and burns...
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <SPIFFS.h>
#include <HTTPClient.h>

WebServer server(80); //create server
WiFiManager wm; //create wifimanager
const char* server_name = "http://api.thingspeak.com/update";
//char server_name[64] = "http://api.thingspeak.com/update";
char api_token[32] = "----------------";//change with your thingspeak api token
// Service API Key

//String apiKey = "----------------"; //node0
//String apiKey = "----------------"; //node1
//String apiKey = "----------------"; //node2
//String apiKey = "----------------"; //node3

String statusNetwork = "--";
//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setupSpiffs() {
  //clean FS, for testing
  // SPIFFS.format();
  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          //strcpy(server_name, json["server_name"]);
          strcpy(api_token, json["api_token"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
}

void setupWM() {
  WiFi.mode(WIFI_AP_STA); // explicitly set mode, esp defaults to STA+AP
  setupSpiffs();
  WiFiManager wm;
  //set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);
  //WiFiManagerParameter custom_server_name("server", "server name", "", 64);
  WiFiManagerParameter custom_api_token("api", "api token", "", 32);
  //wm.addParameter(&custom_server_name);
  wm.addParameter(&custom_api_token);

  wm.setConfigPortalTimeout(180);
  //wm.setSTAStaticIPConfig(IPAddress(10, 0, 1, 1), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));
  wm.setAPStaticIPConfig(IPAddress(10, 0, 1, 1), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));
  //wm.resetSettings();
  if (!wm.autoConnect("Air_Monitor", "green2020")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.restart();// if we still have not connected restart and try all over again
    delay(5000);
  }
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  //strcpy(server_name, custom_server_name.getValue());
  strcpy(api_token, custom_api_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    //json["server_name"]   = server_name;
    json["api_token"]   = api_token;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
    shouldSaveConfig = false;
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.subnetMask());
}
void connectionCheck() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  delay(1000);
}

void send_to_thingspeak(String httpRequestData) {
  HTTPClient http;
  http.begin(server_name);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(httpRequestData);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  if (httpResponseCode == 200) {
    statusNetwork = "OK";
  }
  else {
    statusNetwork = String(httpResponseCode);
  }
  http.end();
}
