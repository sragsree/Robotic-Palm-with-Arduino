#include <PalmControl.h>
#include <iostream>

int main() {
  palm::Receiver receiver;
  const int16_t readings[][5] = {{200,200,200,200,200}, {200,265,330,395,460}, {460,460,460,460,460}};
  for (uint8_t sample = 0; sample < 3; ++sample) {
    uint8_t angles[5], frame[palm::kFrameSize];
    for (uint8_t finger = 0; finger < 5; ++finger)
      angles[finger] = palm::flexAngle(readings[sample][finger], {200, 460});
    palm::encode(sample, angles, frame);
    const uint32_t now = sample * 20;
    for (uint8_t i = 0; i < sizeof(frame); ++i) receiver.feed(frame[i], now);
    std::cout << "sample " << int(sample) << " | servo targets:";
    for (uint8_t finger = 0; finger < 5; ++finger)
      std::cout << ' ' << int(palm::servoAngle(receiver.angle(finger, now), 10, 170));
    std::cout << '\n';
  }
  std::cout << "radio silent 500 ms | servo targets:";
  for (uint8_t finger = 0; finger < 5; ++finger)
    std::cout << ' ' << int(palm::servoAngle(receiver.angle(finger, 540), 10, 170));
  std::cout << "\nHost simulation of real control/protocol code; no physical hardware was exercised.\n";
}
