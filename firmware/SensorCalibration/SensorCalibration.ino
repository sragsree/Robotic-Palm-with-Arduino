// Disconnect the radio. Use Serial Monitor at 9600 baud to measure open/closed readings.
void setup() { Serial.begin(9600); }
void loop() {
  const uint8_t pins[] = {A0, A1, A2, A3, A4};
  for (uint8_t i = 0; i < 5; ++i) {
    if (i) Serial.print(',');
    Serial.print(analogRead(pins[i]));
  }
  Serial.println();
  delay(200);
}
