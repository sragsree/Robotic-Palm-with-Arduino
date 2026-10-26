#include <PalmControl.h>

// Thumb, index, middle, ring, little. Measure each sensor before attaching tendons.
const uint8_t kSensorPins[palm::kFingers] = {A0, A1, A2, A3, A4};
const palm::Calibration kCalibration[palm::kFingers] = {
    {200, 460}, {200, 460}, {200, 460}, {200, 460}, {200, 460}};
uint8_t sequence = 0;
uint32_t lastSendMs = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  const uint32_t now = millis();
  if (uint32_t(now - lastSendMs) < 20) return;
  lastSendMs = now;
  uint8_t angles[palm::kFingers];
  for (uint8_t i = 0; i < palm::kFingers; ++i) {
    uint16_t sum = 0;
    for (uint8_t sample = 0; sample < 4; ++sample) sum += analogRead(kSensorPins[i]);
    angles[i] = palm::flexAngle(sum / 4, kCalibration[i]);
  }
  uint8_t frame[palm::kFrameSize];
  palm::encode(sequence++, angles, frame);
  Serial.write(frame, sizeof(frame));
}
