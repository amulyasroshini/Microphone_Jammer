#include <Arduino.h>
#include <esp_system.h>

// Arduino-ESP32 core 3.x. USB powers the ESP32; GPIO18 is a gate signal,
// not the piezo power supply. See the 5 V circuit specifications.
constexpr uint8_t PWM_PIN = 18;
constexpr uint8_t PWM_BITS = 8;
constexpr uint32_t MIN_HZ = 24000;
constexpr uint32_t MAX_HZ = 26000;
constexpr uint32_t HOP_INTERVAL_MS = 10;

void setup() {
  pinMode(PWM_PIN, OUTPUT);
  digitalWrite(PWM_PIN, LOW);
  if (!ledcAttach(PWM_PIN, 25000, PWM_BITS)) {
    while (true) { delay(1000); }
  }
  if (!ledcWrite(PWM_PIN, 128)) { // approximately 50% duty
    ledcDetach(PWM_PIN);
    pinMode(PWM_PIN, OUTPUT);
    digitalWrite(PWM_PIN, LOW);
    while (true) { delay(1000); }
  }
}

void loop() {
  // Change every 10 ms, not every cycle as in the LTspice PWL example.
  uint32_t requestedHz = MIN_HZ + esp_random() % (MAX_HZ - MIN_HZ + 1);
  if (ledcChangeFrequency(PWM_PIN, requestedHz, PWM_BITS) == 0) {
    ledcWrite(PWM_PIN, 0);
    ledcDetach(PWM_PIN);
    pinMode(PWM_PIN, OUTPUT);
    digitalWrite(PWM_PIN, LOW);
    while (true) { delay(1000); }
  }
  delay(HOP_INTERVAL_MS);
}
