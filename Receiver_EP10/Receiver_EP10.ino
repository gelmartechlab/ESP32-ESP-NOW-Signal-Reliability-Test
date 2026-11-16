#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_err.h>
#include <atomic>

constexpr int LED_PIN = 23;
constexpr uint8_t CHANNEL = 1;

constexpr unsigned long PACKET_TIMEOUT_MS = 1000;
constexpr unsigned long PRINT_INTERVAL_MS = 1000;

// Stores LED state and RSSI together.
// -1 means no new packet is waiting.
std::atomic<int> pendingReading{-1};

int lastState = -1;
int latestRssi = 0;

bool haveSignal = false;

unsigned long lastPacketAt = 0;
unsigned long lastPrintAt = 0;

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

void onReceive(const esp_now_recv_info_t *info,
               const uint8_t *data,
               int length) {
  if (!info || !info->rx_ctrl || !data) {
    return;
  }

  if (length != 1 || data[0] > 1) {
    return;
  }

  const int rssi = info->rx_ctrl->rssi;

  // Encode RSSI and the received state into one value.
  // Copy metadata before the callback returns.
  const int reading =
    ((rssi + 128) << 1) | data[0];

  pendingReading.store(reading);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);
  delay(500);

  Serial.println("\nESP32-D receiver starting");

  // Brief flash to check the external LED wiring.
  digitalWrite(LED_PIN, HIGH);
  delay(300);
  digitalWrite(LED_PIN, LOW);

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

  uint8_t mac[6] = {};

  requireOK(
    esp_wifi_get_mac(WIFI_IF_STA, mac),
    "MAC read"
  );

  Serial.printf(
    "Receiver MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
    mac[0], mac[1], mac[2],
    mac[3], mac[4], mac[5]
  );

  requireOK(
    esp_now_register_recv_cb(onReceive),
    "Receive callback"
  );

  Serial.println("Receiver ready on channel 1");
  Serial.println("Waiting for transmitter...");
}

void loop() {
  const int reading = pendingReading.exchange(-1);
  const unsigned long now = millis();

  if (reading >= 0) {
    const int state = reading & 1;
    latestRssi = (reading >> 1) - 128;

    lastPacketAt = now;
    haveSignal = true;

    if (state != lastState) {
      lastState = state;

      // Update the LED before printing.
      digitalWrite(
        LED_PIN,
        state ? HIGH : LOW
      );

      Serial.printf(
        "RX: %d | LED: %s | RSSI: %d dBm\n",
        state,
        state ? "ON" : "OFF",
        latestRssi
      );

      lastPrintAt = now;
    } else if (
      now - lastPrintAt >= PRINT_INTERVAL_MS
    ) {
      Serial.printf(
        "LIVE | LED: %s | RSSI: %d dBm\n",
        state ? "ON" : "OFF",
        latestRssi
      );

      lastPrintAt = now;
    }
  }

  if (
    haveSignal &&
    now - lastPacketAt >= PACKET_TIMEOUT_MS
  ) {
    digitalWrite(LED_PIN, LOW);

    lastState = -1;
    haveSignal = false;

    Serial.println(
      "No packet for 1 second | LED OFF | RSSI unavailable"
    );
  }

  delay(1);
}
