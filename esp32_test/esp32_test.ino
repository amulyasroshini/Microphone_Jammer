// ESP32 Arduino core 3.x: 25 kHz one-piezo experimental driver
// GPIO18 drives the gate; the 3V3 power pin supplies the load (no code needed).
const int PWM_PIN = 18;
void setup() {
  pinMode(PWM_PIN, OUTPUT);
  digitalWrite(PWM_PIN, LOW);
  if (!ledcAttach(PWM_PIN, 25000, 8)) {
    while (true) { delay(1000); }
  }
  ledcWrite(PWM_PIN, 128); // 50% duty on 8-bit timer
}
void loop() { delay(1000); }


