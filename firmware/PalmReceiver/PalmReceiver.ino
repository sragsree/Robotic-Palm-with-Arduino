#include <Servo.h>
#include <PalmControl.h>

// Thumb, index, middle, ring, little. Mechanical limits must be measured per build.
const uint8_t kServoPins[palm::kFingers] = {2, 3, 4, 5, 6};
const uint8_t kOpenAngles[palm::kFingers] = {10, 10, 10, 10, 10};
const uint8_t kClosedAngles[palm::kFingers] = {170, 170, 170, 170, 170};
Servo servos[palm::kFingers];
uint8_t currentAngles[palm::kFingers];
palm::Receiver receiver;
uint32_t lastMoveMs = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  for (uint8_t i = 0; i < palm::kFingers; ++i) {
    currentAngles[i] = kOpenAngles[i];
    servos[i].write(currentAngles[i]);
    servos[i].attach(kServoPins[i]);
  }
}

void loop() {
  const uint32_t now = millis();
  while (Serial.available() > 0) receiver.feed(uint8_t(Serial.read()), now);
  digitalWrite(LED_BUILTIN, receiver.connected(now) ? HIGH : LOW);
  if (uint32_t(now - lastMoveMs) < 20) return;
  lastMoveMs = now;
  for (uint8_t i = 0; i < palm::kFingers; ++i) {
    const uint8_t target = palm::servoAngle(receiver.angle(i, now), kOpenAngles[i], kClosedAngles[i]);
    currentAngles[i] = palm::approach(currentAngles[i], target, 3);
    servos[i].write(currentAngles[i]);
  }
}
