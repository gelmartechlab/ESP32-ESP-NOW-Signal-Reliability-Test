#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_err.h>
#include <cstring>

constexpr int BUTTON_PIN = 4;
constexpr uint8_t CHANNEL = 1;

constexpr unsigned long DEBOUNCE_MS = 30;
constexpr unsigned long RESEND_MS = 250;

// Replace with your receiver's station MAC if different.
const uint8_t RECEIVER_MAC[6] = {
  0x4C, 0xC3, 0x82, 0x0D, 0xAB, 0xF0
};

bool lastRawPressed = false;
bool stablePressed = false;

unsigned long lastInputChange = 0;
unsigned long lastSendAt = 0;

void requireOK(esp_err_t result, const char *step) {
  if (result != ESP_OK) {
    Serial.printf(
      "%s failed: %s\n",
      step,
      esp_err_to_name(result)
    );

    while (true) {
      delay(1000);
    }
  }
}

void sendState() {
  const uint8_t command = stablePressed ? 1 : 0;

  const esp_err_t result = esp_now_send(
    RECEIVER_MAC,
    &command,
    sizeof(command)
  );

  if (result != ESP_OK) {
    Serial.printf(
      "Send queue failed: %s\n",
      esp_err_to_name(result)
    );
  }

  lastSendAt = millis();
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  delay(500);

  Serial.println("\nESP32-S3 transmitter starting");

  if (!WiFi.mode(WIFI_STA)) {
    Serial.println("Wi-Fi start failed");

    while (true) {
      delay(1000);
    }
  }

  requireOK(
    esp_wifi_set_channel(
      CHANNEL,
      WIFI_SECOND_CHAN_NONE
    ),
    "Channel setup"
  );

  requireOK(
    esp_now_init(),
    "ESP-NOW setup"
  );

  esp_now_peer_info_t peer = {};

  std::memcpy(
    peer.peer_addr,
    RECEIVER_MAC,
    sizeof(RECEIVER_MAC)
  );

  peer.channel = CHANNEL;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;

  requireOK(
    esp_now_add_peer(&peer),
    "Add receiver"
  );

  lastRawPressed = digitalRead(BUTTON_PIN) == LOW;
  stablePressed = lastRawPressed;
  lastInputChange = millis();

  Serial.println("Transmitter ready on channel 1");
  Serial.println(
    "GPIO4 button: released HIGH, pressed LOW"
  );

  sendState();
}

void loop() {
  const unsigned long now = millis();
  const bool rawPressed =
    digitalRead(BUTTON_PIN) == LOW;

  if (rawPressed != lastRawPressed) {
    lastRawPressed = rawPressed;
    lastInputChange = now;
  }

  const bool changed =
    rawPressed != stablePressed &&
    now - lastInputChange >= DEBOUNCE_MS;

  if (changed) {
    stablePressed = rawPressed;

    // Send before printing to reduce Serial-related delay.
    sendState();

    Serial.println(
      stablePressed
        ? "Button PRESSED | Command: 1"
        : "Button RELEASED | Command: 0"
    );
  } else if (now - lastSendAt >= RESEND_MS) {
    // Refresh the current button state every 250 ms.
    sendState();
  }

  delay(1);
}
