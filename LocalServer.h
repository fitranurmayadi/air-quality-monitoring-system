
/**************************************************************************************************************************************************************************/
const char HTTP[]             PROGMEM =
  "<!DOCTYPE html><html>"
  "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
  "<link rel=\"icon\" href=\"data:,\">"
  "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"
  ".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;"
  "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}"
  ".button2 {background-color: #77878A;}</style></head>"
  "<body><h1>Press Button To Reset Settings</h1>"
  "<p><a href=\"/reset\"><button class=\"button\">Reset</button></a></p>"
  "</body></html>";

void handle_OnConnect() {
  server.send(200, "text/html", HTTP);
}

void handle_reset() {
  Serial.println("Erase settings and restart ...");
  delay(1000);
  wm.resetSettings();
  ESP.restart();
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}
