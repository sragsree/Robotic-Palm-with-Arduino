#include <PalmControl.h>
#include <cassert>
#include <iostream>

static bool feed(palm::Receiver& receiver, const uint8_t* bytes, size_t count, uint32_t now) {
  bool accepted = false;
  for (size_t i = 0; i < count; ++i) accepted = receiver.feed(bytes[i], now) || accepted;
  return accepted;
}

int main() {
  const uint8_t check[] = {'1','2','3','4','5','6','7','8','9'};
  assert(palm::crc8(check, sizeof(check)) == 0xF4);
  for (int raw = 0; raw <= 1023; ++raw) {
    assert(palm::flexAngle(raw, {200, 460}) <= 180);
    assert(palm::flexAngle(raw, {460, 200}) <= 180);
  }
  assert(palm::flexAngle(200, {200, 460}) == 0);
  assert(palm::flexAngle(330, {200, 460}) == 90);
  assert(palm::flexAngle(460, {200, 460}) == 180);
  assert(palm::flexAngle(330, {460, 200}) == 90);
  assert(palm::flexAngle(460, {460, 200}) == 0);
  assert(palm::flexAngle(200, {460, 200}) == 180);
  assert(palm::flexAngle(500, {200, 200}) == 0);

  uint8_t angles[] = {0, 45, 90, 135, 180};
  uint8_t frame[palm::kFrameSize];
  palm::encode(255, angles, frame);
  palm::Receiver receiver;
  assert(!receiver.connected(0));
  assert(receiver.angle(0, 0) == 0);
  assert(!feed(receiver, frame, 8, 10));
  assert(receiver.feed(frame[8], 10));
  assert(receiver.sequence() == 255);
  for (uint8_t i = 0; i < palm::kFingers; ++i) assert(receiver.angle(i, 10) == angles[i]);
  assert(receiver.connected(509));
  assert(!receiver.connected(510));
  assert(receiver.angle(4, 510) == 0);
  assert(receiver.angle(5, 10) == 0);

  // Every one-bit frame corruption must be rejected and must not keep a link alive.
  for (unsigned bit = 0; bit < sizeof(frame) * 8; ++bit) {
    palm::Receiver damaged;
    uint8_t bad[palm::kFrameSize];
    for (unsigned i = 0; i < sizeof(frame); ++i) bad[i] = frame[i];
    bad[bit / 8] ^= uint8_t(1 << (bit % 8));
    assert(!feed(damaged, bad, sizeof(bad), 100));
    assert(!damaged.connected(100));
    assert(feed(damaged, frame, sizeof(frame), 101));
  }

  // Dropped bytes, inserted noise, and a truncated old frame recover on the next frame.
  for (unsigned missing = 0; missing < sizeof(frame); ++missing) {
    palm::Receiver shifted;
    for (unsigned i = 0; i < sizeof(frame); ++i) if (i != missing) shifted.feed(frame[i], 100);
    assert(feed(shifted, frame, sizeof(frame), 101));
  }
  const uint8_t noise[] = {0, 255, 0xA5, 0x5A, 12, 0xA5};
  feed(receiver, noise, sizeof(noise), 600);
  assert(!receiver.connected(600));
  assert(feed(receiver, frame, sizeof(frame), 601));

  uint8_t invalid[palm::kFrameSize];
  palm::encode(1, angles, invalid);
  invalid[3] = 181;
  invalid[8] = palm::crc8(invalid, 8);
  assert(!feed(receiver, invalid, sizeof(invalid), 1000));
  assert(!receiver.connected(1101));

  palm::encode(0, angles, frame);
  assert(feed(receiver, frame, sizeof(frame), 0xFFFFFFF0u));
  assert(receiver.sequence() == 0);
  assert(receiver.connected(0x10u));  // millis() wraparound
  assert(!receiver.connected(0x1E4u));
  assert(feed(receiver, frame, sizeof(frame), 100));
  assert(!receiver.connected(600));
  assert(!receiver.connected(100));  // another full wrap must not revive stale data
  assert(receiver.angle(4, 100) == 0);
  assert(feed(receiver, frame, sizeof(frame), 101));
  assert(receiver.angle(4, 101) == 180);
  assert(palm::servoAngle(0, 10, 170) == 10);
  assert(palm::servoAngle(90, 10, 170) == 90);
  assert(palm::servoAngle(180, 10, 170) == 170);
  assert(palm::servoAngle(0, 170, 10) == 170);
  assert(palm::servoAngle(180, 170, 10) == 10);
  assert(palm::servoAngle(255, 255, 255) <= 180);
  assert(palm::approach(10, 170, 3) == 13);
  assert(palm::approach(170, 10, 3) == 167);
  assert(palm::approach(10, 12, 3) == 12);
  assert(palm::approach(12, 10, 3) == 10);
  assert(palm::approach(10, 10, 3) == 10);
  std::cout << "PASS: calibration, CRC, five channels, corruption, resynchronization, timeout, rollover, limits, slew\n";
}
