#include <WiFi.h>       // WiFi connection for ESP32
#include <HTTPClient.h> // HTTP POST for WLED

const char* WIFI_SSID = "ARM-band";
const char* WIFI_PASS = "vivawed6";

const char* WLED_HOST = "4.3.2.1";

const int MATRIX_W = 36; // width (columns)
const int MATRIX_H = 8;  // height (rows)

const int BUTTON_PINS[] = {3, 4, 5, 6, 7, 21};
const int BUTTON_COUNT = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);

// ---- Button state struct (for debounce) ----
struct ButtonState {
  int pin;                   // GPIO number
  int lastReading;           // last raw read
  int stableState;           // debounced stable state
  unsigned long lastChange;  // last time it changed
};

ButtonState buttons[BUTTON_COUNT] = {
  {3, HIGH, HIGH, 0},   // GPIO3  -> GREEN toggle
  {4, HIGH, HIGH, 0},   // GPIO4  -> BLUE toggle
  {5, HIGH, HIGH, 0},   // GPIO5  -> PURPLE toggle
  {6, HIGH, HIGH, 0},   // GPIO6  -> CYAN toggle
  {7, HIGH, HIGH, 0},   // GPIO7  -> WHITE toggle
  {21, HIGH, HIGH, 0}   // GPIO21 -> RESET (only red)
};

const uint8_t BASE_RED[3] = {255, 0, 0};

const uint8_t EXTRA_COLORS[5][3] = {
  {0,   255, 0},   // green
  {0,   0,   255}, // blue
  {255, 0,   255}, // purple
  {0,   255, 255}, // cyan
  {255, 255, 255}  // white
};

// ---- Which extra colors are active ----
bool activeExtra[5] = {false, false, false, false, false};

// ---- WLED API URL ----
String url() {
  return String("http://") + WLED_HOST + "/json/state";
}

// ---- Send JSON POST to WLED ----
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
  h.end();
}

// ---- Debounced button press ----
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
      if (b.stableState == LOW) return true; 
    }
  }
  return false;
}

// ---- Apply equal segments based on active colors ----
void applySegments(uint8_t brightness = 128) {
  const int MAX_SEGMENTS = 6; 
  uint8_t colors[6][3];
  int count = 1;

  // First color: red
  colors[0][0] = BASE_RED[0];
  colors[0][1] = BASE_RED[1];
  colors[0][2] = BASE_RED[2];

  // Add extra colors if active
  for (int i = 0; i < 5; i++) {
    if (activeExtra[i]) {
      colors[count][0] = EXTRA_COLORS[i][0];
      colors[count][1] = EXTRA_COLORS[i][1];
      colors[count][2] = EXTRA_COLORS[i][2];
      count++;
    }
  }

  // Equal split based on active color count
  int baseW = MATRIX_W / count;
  int rem   = MATRIX_W % count;

  String json = "{\"on\":true,\"bri\":";
  json += brightness;
  json += ",\"seg\":[";

  int startX = 0;
  for (int i = 0; i < MAX_SEGMENTS; i++) {
    if (i < count) {
      // Active segment
      int w = baseW + (i < rem ? 1 : 0);
      int stopX = startX + w;

      json += "{\"id\":";
      json += i;
      json += ",\"on\":true";
      json += ",\"startY\":0,\"stopY\":";
      json += MATRIX_H;
      json += ",\"start\":";
      json += startX;
      json += ",\"stop\":";
      json += stopX;
      json += ",\"col\":[[";
      json += colors[i][0];
      json += ",";
      json += colors[i][1];
      json += ",";
      json += colors[i][2];
      json += "]]}";

      startX = stopX;
    } else {
      // Unused segment -> turn off
      json += "{\"id\":";
      json += i;
      json += ",\"on\":false,\"start\":0,\"stop\":0,\"startY\":0,\"stopY\":0,\"col\":[[0,0,0]]}";
    }

    if (i + 1 < MAX_SEGMENTS) json += ",";
  }

  json += "]}";
  post(json);
}

void setup() {
  Serial.begin(115200);

  // Buttons as input with pullup
  for (int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }

  // WiFi connect
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
  }

  // Start: 100% red
  applySegments();
}

void loop() {
  for (int i = 0; i < 5; i++) {
    if (buttonPressed(buttons[i])) {
      activeExtra[i] = !activeExtra[i];
      applySegments();
    }
  }

  if (buttonPressed(buttons[5])) {
    for (int i = 0; i < 5; i++) activeExtra[i] = false;
    applySegments();
  }
}