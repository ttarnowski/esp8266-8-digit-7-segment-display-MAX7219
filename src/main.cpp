#include <Arduino.h>
#include <LedController.hpp>

#define LED_CONTROL_CLK_PIN D5
#define LED_CONTROL_DATA_PIN D6
#define LED_CONTROL_CS_PIN D7
#define LED_CONTROL_NUM_SEGMENTS 1

LedController ledController(LED_CONTROL_DATA_PIN, LED_CONTROL_CLK_PIN,
                            LED_CONTROL_CS_PIN, LED_CONTROL_NUM_SEGMENTS);

void setup() {
  Serial.begin(115200);
  delay(1000);

  ledController.activateAllSegments();
  ledController.setIntensity(15);
  ledController.clearMatrix();

  delay(1000);

  for (unsigned int i = 0; i < 8; i++) {
    ledController.setChar(0, i, (char)8 - i, false);
  }
}

void loop() {}