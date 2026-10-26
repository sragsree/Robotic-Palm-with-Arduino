#ifndef PALM_CONTROL_H
#define PALM_CONTROL_H

#include <stdint.h>
#include <stddef.h>

namespace palm {
const uint8_t kFingers = 5;
const uint8_t kFrameSize = 9;
const uint32_t kLinkTimeoutMs = 500;

struct Calibration {
  int16_t open;
  int16_t closed;
};

// Reversed sensors are supported; an invalid calibration fails open.
inline uint8_t flexAngle(int16_t reading, Calibration calibration) {
  const int32_t span = int32_t(calibration.closed) - calibration.open;
  if (span == 0) return 0;
  int32_t angle = (int32_t(reading) - calibration.open) * 180 / span;
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;
  return uint8_t(angle);
}

// CRC-8/ATM: polynomial 0x07, initial value 0, no reflection or final XOR.
inline uint8_t crc8(const uint8_t* bytes, size_t length) {
  uint8_t crc = 0;
  for (size_t i = 0; i < length; ++i) {
    crc ^= bytes[i];
    for (uint8_t bit = 0; bit < 8; ++bit)
      crc = uint8_t((crc & 0x80) ? (crc << 1) ^ 0x07 : crc << 1);
  }
  return crc;
}

inline void encode(uint8_t sequence, const uint8_t* angles, uint8_t* frame) {
  frame[0] = 0xA5;
  frame[1] = 0x5A;
  frame[2] = sequence;
  for (uint8_t i = 0; i < kFingers; ++i)
    frame[3 + i] = angles[i] > 180 ? 180 : angles[i];
  frame[8] = crc8(frame, 8);
}

class Receiver {
 public:
  Receiver() : count_(0), lastValidMs_(0), received_(false), sequence_(0) {
    for (uint8_t i = 0; i < kFingers; ++i) angles_[i] = 0;
  }

  // A sliding window resynchronizes after insertion, deletion, or partial frames.
  bool feed(uint8_t byte, uint32_t now) {
    if (count_ == kFrameSize) {
      for (uint8_t i = 1; i < kFrameSize; ++i) window_[i - 1] = window_[i];
      --count_;
    }
    window_[count_++] = byte;
    if (count_ != kFrameSize || window_[0] != 0xA5 || window_[1] != 0x5A ||
        crc8(window_, 8) != window_[8]) return false;
    for (uint8_t i = 0; i < kFingers; ++i)
      if (window_[3 + i] > 180) return false;
    for (uint8_t i = 0; i < kFingers; ++i) angles_[i] = window_[3 + i];
    sequence_ = window_[2];
    lastValidMs_ = now;
    received_ = true;
    count_ = 0;
    return true;
  }

  bool connected(uint32_t now) {
    // Latch expiration until a new valid packet. Without this, stale targets could
    // reappear for 500 ms after millis() completes a full 32-bit wraparound.
    if (received_ && uint32_t(now - lastValidMs_) >= kLinkTimeoutMs)
      received_ = false;
    return received_;
  }

  uint8_t angle(uint8_t finger, uint32_t now) {
    return finger < kFingers && connected(now) ? angles_[finger] : 0;
  }

  uint8_t sequence() const { return sequence_; }

 private:
  uint8_t window_[kFrameSize];
  uint8_t angles_[kFingers];
  uint8_t count_;
  uint32_t lastValidMs_;
  bool received_;
  uint8_t sequence_;
};

inline uint8_t servoAngle(uint8_t flex, uint8_t open, uint8_t closed) {
  if (flex > 180) flex = 180;
  if (open > 180) open = 180;
  if (closed > 180) closed = 180;
  return uint8_t(int16_t(open) + int32_t(int16_t(closed) - open) * flex / 180);
}

inline uint8_t approach(uint8_t current, uint8_t target, uint8_t step) {
  if (current < target)
    return target - current > step ? uint8_t(current + step) : target;
  return current - target > step ? uint8_t(current - step) : target;
}
}  // namespace palm
#endif
