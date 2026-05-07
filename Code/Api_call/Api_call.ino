#include <WiFi.h>
#include <HTTPClient.h>



const char* WIFI_SSID = "ARM-band";
const char* WIFI_PASS = "vivawed6";

const char* WLED_HOST = "4.3.2.1";
const uint16_t WLED_PORT = 80;

// Build the full URL to a WLED HTTP endpoint
String wledUrl(const char* path) {
  return String("http://") + WLED_HOST + ":" + String(WLED_PORT) + path;
}

// Send a JSON state update to WLED and print the result to Serial
bool wledPostStateJson(const String& json) {
  // Only attempt HTTP if we are connected to Wi-Fi
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  const String url = wledUrl("/json/state");

  // Open HTTP connection and send JSON body
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  int code = http.POST((uint8_t*)json.c_str(), json.length());
  String resp = http.getString();
  http.end();

  // Debug: show whether WLED accepted the request
  Serial.print("POST ");
  Serial.print(url);
  Serial.print(" -> HTTP ");
  Serial.println(code);

  if (resp.length()) {
    Serial.print("Resp: ");
    Serial.println(resp);
  }

  return code >= 200 && code < 300;
}

// Connect to the target Wi-Fi (WLED AP or your router)
void connectToAp() {
  Serial.print("Connecting to AP: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait up to ~20 seconds for connection
  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
    if (millis() - start > 20000) {
      Serial.println("\nWiFi connect TIMEOUT");
      return;
    }
  }

  // Connected: print IP details for troubleshooting
  Serial.println("\nWiFi connected!");
  Serial.print("STA IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());
}

void setup() {
 
  Serial.begin(115200);
  delay(1500);
  Serial.println("\nBOOT OK - starting WiFi connect...");

  connectToAp();
}

void loop() {
  // If Wi-Fi drops, try to reconnect instead of sending HTTP
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, retrying...");
    connectToAp();
    delay(1000);
    return;
  }
LED
  static uint8_t step = 0;
  String json;

  if (step % 3 == 0) {
    json = R"({"on":true,"bri":128,"seg":[{"col":[[255,0,0]]}]})"; 
  } else if (step % 3 == 1) {
    json = R"({"on":true,"bri":128,"seg":[{"col":[[0,255,0]]}]})"; 
  } else {
    json = R"({"on":true,"bri":128,"seg":[{"col":[[0,0,255]]}]})"; 
  }

  //  push the new state to WLED
  wledPostStateJson(json);
  step++;

  
  delay(5000);
}