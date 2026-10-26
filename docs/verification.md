# Software verification record

Run date: 2026-10-25 (America/Los_Angeles). Baseline: `bee07371434bceba0ee1aa708908902f07b6a3ce`. Repair branch: `codex/operational-readme`.

| Check | Result |
|---|---|
| `make test` | Passed: CRC standard vector, full ADC range, normal/reversed/zero-span calibration, all five channels, incomplete frames, all 72 single-bit frame corruptions, each dropped-byte position, noisy prefixes, invalid angle with valid CRC, timeout, millis rollover, persistent timeout across another full wrap, fresh-frame recovery, servo limits and slew |
| `make demo` | Expected independent setpoints and timeout-to-open output |
| `make compile` | Transmitter, receiver, and sensor-calibration sketches compile for `arduino:avr:uno` |
| `git diff --check` | Passed |
| Independent source review | Found and fixed stale targets reviving after full timer wrap; separate repro confirms expiration remains latched until a new valid frame |

Toolchain: Apple Clang C++11 with `-Wall -Wextra -Werror -pedantic`; Arduino CLI 1.3.1; Arduino AVR core 1.8.6; Servo 1.2.2.

No boards, radios, servos, or physical palm were connected. No firmware was flashed. [Hardware acceptance](bench-acceptance.md) remains pending. CI workflow is included but no remote CI execution is claimed.

## Re-run before publication

On 2026-10-25, the preserved Git clone was rebuilt using `make test demo` and `make compile`. Host regressions and the demo passed; all three Uno builds passed again. Flash/RAM usage remained 2,106/215 bytes for the transmitter, 3,658/283 for the receiver, and 1,958/192 for calibration. Build outputs remain ignored. No physical hardware was connected or flashed.
