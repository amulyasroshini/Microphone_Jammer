#include <Arduino.h>
#include <esp_system.h>

// Arduino-ESP32 core 3.x. GPIO19 monitors the 3.3 V GATE signal only.
// Never connect GPIO19 to the 5 V supply, drain, or piezo terminals.
constexpr uint8_t PWM_PIN = 18;
constexpr uint8_t MONITOR_PIN = 19;  // Verify GPIO19 exists on your ESP32 board.
constexpr uint8_t PWM_BITS = 8;
constexpr uint32_t MIN_HZ = 24000;
constexpr uint32_t MAX_HZ = 26000;
constexpr uint32_t HOP_INTERVAL_MS = 10;
constexpr uint32_t REPORT_INTERVAL_MS = 1000;

volatile uint32_t pulseCount = 0;
portMUX_TYPE countMux = portMUX_INITIALIZER_UNLOCKED;
uint32_t lastHopMs;
uint32_t lastReportMs;
uint32_t lastFrequency;

void IRAM_ATTR countPulse() {
  portENTER_CRITICAL_ISR(&countMux);
  ++pulseCount;
  portEXIT_CRITICAL_ISR(&countMux);
}

void stopOutput(const char* message) {
  ledcWrite(PWM_PIN, 0);
  ledcDetach(PWM_PIN);
  pinMode(PWM_PIN, OUTPUT);
  digitalWrite(PWM_PIN, LOW);
  Serial.println(message);
  while (true) { delay(1000); }
}

void setup() {
  Serial.begin(115200);
  pinMode(PWM_PIN, OUTPUT);
  digitalWrite(PWM_PIN, LOW);
  pinMode(MONITOR_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(MONITOR_PIN), countPulse, RISING);
  if (!ledcAttach(PWM_PIN, 25000, PWM_BITS)) {
    Serial.println("ERROR: PWM setup failed; output remains LOW.");
    while (true) { delay(1000); }
  }
  if (!ledcWrite(PWM_PIN, 128)) {
    stopOutput("ERROR: PWM duty setup failed; output disabled.");
  }
  lastFrequency = 25000;
  lastHopMs = lastReportMs = millis();
  Serial.println("Connect GPIO19 to GPIO18/gate node; do not connect it to drain.");
}

void loop() {
  uint32_t now = millis();
  if ((uint32_t)(now - lastHopMs) >= HOP_INTERVAL_MS) {
    lastHopMs = now;
    uint32_t request = MIN_HZ + esp_random() % (MAX_HZ - MIN_HZ + 1);
    uint32_t actual = ledcChangeFrequency(PWM_PIN, request, PWM_BITS);
    if (actual == 0) stopOutput("ERROR: frequency change failed; output disabled.");
    lastFrequency = actual;
  }
  uint32_t elapsed = (uint32_t)(now - lastReportMs);
  if (elapsed >= REPORT_INTERVAL_MS) {
    portENTER_CRITICAL(&countMux);
    uint32_t pulses = pulseCount;
    pulseCount = 0;
    portEXIT_CRITICAL(&countMux);
    lastReportMs = now;
    uint32_t averageHz = (uint32_t)(((uint64_t)pulses * 1000ULL) / elapsed);
    Serial.printf("GPIO19 counted %lu pulses in %lu ms (~%lu Hz); last configured: %lu Hz\n",
                  (unsigned long)pulses, (unsigned long)elapsed,
                  (unsigned long)averageHz, (unsigned long)lastFrequency);
  }
  delay(1);
}
