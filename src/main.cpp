#include <Arduino.h>
#include <DHTesp.h>
#include <LedController.hpp>
#include <Timer.hpp>
#include <ESP8266WiFiMulti.h>

#define TEMP_SENSOR_PIN D1
#define LED_CONTROL_CLK_PIN D5
#define LED_CONTROL_DATA_PIN D6
#define LED_CONTROL_CS_PIN D7
#define LED_CONTROL_NUM_SEGMENTS 2

LedController ledController(LED_CONTROL_DATA_PIN, LED_CONTROL_CLK_PIN,
                            LED_CONTROL_CS_PIN, LED_CONTROL_NUM_SEGMENTS);
DHTesp dht;
Timer timer;

void setClock(std::function<void(bool)> onClockSet,
              unsigned long timeoutMs = 10000) {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.println("Waiting for NTP time sync: ");

  timer.setOnLoopUntil(
      [onClockSet]() {
        time_t now = time(nullptr);
        if (now >= 8 * 3600 * 2) {
          Serial.println("");
          struct tm timeinfo;
          gmtime_r(&now, &timeinfo);
          Serial.print("Current time: ");
          Serial.print(asctime(&timeinfo));
          onClockSet(true);
          return true;
        }

        return false;
      },
      [onClockSet]() { onClockSet(false); }, timeoutMs);
}

void display(const char *value, unsigned int segment = 0) {
  ledController.clearSegment(segment);

  uint8_t dotsNumber = 0;
  uint8_t length = (uint8_t)strlen(value);

  for (uint8_t i = 0; i < length; i++) {
    if (value[i] == '.') {
      dotsNumber++;
    }
  }

  length = length - dotsNumber > 8 ? 8 + dotsNumber : length;

  uint8_t i = 0;
  bool withDot = false;
  uint8_t dotShift = 0;

  while (i < length) {
    uint8_t key = length - i - 1;
    uint8_t pos = i - dotShift;

    if (value[key] == '.') {
      withDot = true;
      dotShift++;
    } else {
      ledController.setChar(segment, pos, value[key], withDot);
      if (withDot) {
        withDot = false;
      }
    }

    i++;
  }
}

void syncTime() {
  display("--------", 1);

  setClock(
      [](bool success) {
        if (!success) {
          Serial.println("could not synchronize the time");
          syncTime();
          return;
        }

        Serial.println("time synchronized");

        timer.setInterval(
            []() {
              time_t now = time(nullptr);
              struct tm timeinfo;
              gmtime_r(&now, &timeinfo);

              char value[16];
              sprintf(value, "%02d .%02d .%02d", timeinfo.tm_hour,
                      timeinfo.tm_min, timeinfo.tm_sec);

              Serial.println(value);
              display(value, 1);
            },
            1000);
      },
      30 * 1000);
}

ESP8266WiFiMulti wifiMulti;

void setup() {
  Serial.begin(115200);
  
  wifiMulti.addAP("YOUR_SSID", "YOUR_PASSWORD");

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  dht.setup(TEMP_SENSOR_PIN, DHTesp::DHT11);

  ledController.activateAllSegments();
  ledController.setIntensity(15);
  ledController.clearMatrix();

  delay(1000);

  timer.setInterval(
      []() {
        timer.setTimeout(
            []() {
              float humidity = dht.getHumidity();
              float temperature = dht.getTemperature();

              char value[16];
              sprintf(value, "%02.2f %02.1f", temperature, humidity);

              Serial.println(value);
              display(value);
            },
            dht.getMinimumSamplingPeriod());
      },
      2000);

  syncTime();
}

void loop() { timer.tick(); }