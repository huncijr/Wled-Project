#include <WiFi.h>
#include <HTTPClient.h>

const char* WIFI_SSID = "ARM-band";
const char* WIFI_PASS = "vivawed6";
const char* WLED_HOST  = "4.3.2.1";

const int LED_COUNT = 288; // 8x36 from repo

const int BUTTON_PINS[] = {3, 4, 5, 6, 7, 21};
const int BUTTON_COUNT = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);

struct ButtonState {
  int pin;
  int lastReading;
  int stableState;
  unsigned long lastChange;
};

ButtonState buttons[BUTTON_COUNT] = {
  {3, HIGH, HIGH, 0},
  {4, HIGH, HIGH, 0},
  {5, HIGH, HIGH, 0},
  {6, HIGH, HIGH, 0},
  {7, HIGH, HIGH, 0},
  {21, HIGH, HIGH, 0}
};

String url() {
  return String("http://") + WLED_HOST + "/json/state";
}

void post(const String& j) {
  HTTPClient h;
  if (!h.begin(url())) {
    Serial.println("http.begin() failed");
    return;
  }

  h.addHeader("Content-Type", "application/json");
  int code = h.POST((uint8_t*)j.c_str(), j.length());

  Serial.print("HTTP ");
  Serial.println(code);

  if (code > 0) {
    Serial.println(h.getString());
  } else {
    Serial.println(h.errorToString(code));
  }

  h.end();
}

void setFullColor(uint8_t r, uint8_t g, uint8_t b) {
  String json =
    "{\"on\":true,\"bri\":128,\"seg\":["
      "{\"id\":0,\"start\":0,\"stop\":" + String(LED_COUNT) + ",\"col\":[[" +
      String(r) + "," + String(g) + "," + String(b) + "]]}"
    "]}";
  post(json);
}

bool buttonPressed(ButtonState &b) {
  const unsigned long debounceMs = 50;
  int reading = digitalRead(b.pin);

  if (reading != b.lastReading) {
    b.lastChange = millis();
    b.lastReading = reading;
  }

  if ((millis() - b.lastChange) > debounceMs) {
    if (reading != b.stableState) {
      b.stableState = reading;
      if (b.stableState == LOW) {
        return true; // pressed
      }
    }
  }

  return false;
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.println("Connecting WiFi...");
  }
  Serial.println("WiFi connected");

  // start with full red
  setFullColor(255, 0, 0);
}

void loop() {
  if (buttonPressed(buttons[0])) { // GPIO3
    Serial.println("GPIO3 -> YELLOW");
    setFullColor(255, 255, 0);
  }
  if (buttonPressed(buttons[1])) { // GPIO4
    Serial.println("GPIO4 -> GREEN");
    setFullColor(0, 255, 0);
  }
  if (buttonPressed(buttons[2])) { // GPIO5
    Serial.println("GPIO5 -> BLUE");
    setFullColor(0, 0, 255);
  }
  if (buttonPressed(buttons[3])) { // GPIO6
    Serial.println("GPIO6 -> PURPLE");
    setFullColor(255, 0, 255);
  }
  if (buttonPressed(buttons[4])) { // GPIO7
    Serial.println("GPIO7 -> CYAN");
    setFullColor(0, 255, 255);
  }
  if (buttonPressed(buttons[5])) { // GPIO21
    Serial.println("GPIO21 -> WHITE");
    setFullColor(255, 255, 255);
  }
}