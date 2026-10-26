# Physical bench acceptance

Status: not executed in the software repair session. Host tests and Uno compiler output are separate evidence. Record board/radio/servo models, supply ratings, calibration values, firmware commit, date, and observations when running this procedure.

1. Inspect polarity, voltage regulation, logic levels, ground continuity, and mechanical clearance with power off. Start with servo horns/tendons unloaded.
2. Run the sensor-calibration sketch. Verify five independent channels and record stable open/closed readings. Incorrect/disconnected wiring must be repaired; software cannot distinguish every disconnected-sensor condition from a valid extreme reading.
3. Configure paired radios in transparent mode. Prove byte transport at 9600 8N1 without servos attached, then install the production sketches on both boards.
4. Power receiver with transmitter off. Confirm commanded open position; attach/test one unloaded servo at a time. Ensure endpoints do not bind.
5. Flex each glove finger separately. Verify only its matching servo changes. Test open hand, closed hand, and mixed flexion. Record observed motion rather than inferring it from LED status.
6. Turn off transmitter power. Receiver LED should extinguish after 500 ms without a valid frame; servos should move gradually toward configured open positions. Reconnect and verify recovery.
7. Repeat with mechanical linkage attached under a controlled light load, monitoring current and binding. Stop on overheating, stall, or unexpected movement. Keep physical power disconnect available.
8. Record radio range/reliability only for measured conditions. Do not extrapolate to industrial, medical, or safety-critical use.

| Check | Expected | Observed | Pass/fail |
|---|---|---|---|
| Startup, no link | Open target; LED off | Not measured | Pending |
| Five independent channels | Matching finger motion only | Not measured | Pending |
| Mixed pose | Five independent flexion positions | Not measured | Pending |
| Link interruption | Timeout and gradual return to open | Not measured | Pending |
| Link recovery | New valid frame resumes targets | Not measured | Pending |
| Mechanical travel/current | No binding; rated load/current | Not measured | Pending |
