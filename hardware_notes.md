# Hardware Integration Notes

## TB6612FNG Motor Driver

- Use two PWM outputs from the TM4C1294XL:
  - `PWMA` for left motor speed
  - `PWMB` for right motor speed
- Use four GPIO direction pins:
  - `AIN1`, `AIN2` for left motor direction
  - `BIN1`, `BIN2` for right motor direction
- Use one GPIO for `STBY`:
  - drive high to enable the chip
  - drive low to force standby/stop
- Recommended command mapping:
  - `CMD_FORWARD`: both motors forward
  - `CMD_REVERSE`: both motors reverse
  - `CMD_LEFT`: left motor slower or stopped, right motor forward
  - `CMD_RIGHT`: right motor slower or stopped, left motor forward
  - `CMD_STOP`: PWM to 0
  - `CMD_BRAKE`: both direction pins same state per channel and PWM as needed

## HC-05 Bluetooth Module

- Default UART settings are commonly:
  - `9600 baud`
  - `8 data bits`
  - `no parity`
  - `1 stop bit`
- TM4C TX can usually connect directly to HC-05 RX only if voltage level is safe for the module revision.
- TM4C RX should read HC-05 TX directly.
- Minimum driver plan:
  - initialize one UART peripheral
  - receive bytes into a small packet buffer
  - parse motion, speed, and mode from the packet
  - update `WirelessCommand`

## HC-SR04 Ultrasonic Sensor

- Trigger sequence:
  - set `TRIG` low briefly
  - set `TRIG` high for `10 us`
  - set `TRIG` low
- Measure the width of the `ECHO` pulse.
- Distance formula:
  - `distance_cm = echo_us / 58.0`
- Important integration note:
  - `ECHO` is a `5V` signal on many HC-SR04 modules, so level shifting or protection is needed before feeding a TM4C input pin.
- Good implementation options:
  - timer input capture for pulse width
  - or a GPIO/timer polling method first, then move to capture mode later

## Recommended Next Hardware Steps

1. Pick exact TM4C pins for:
   - `PWMA`, `PWMB`
   - `AIN1`, `AIN2`, `BIN1`, `BIN2`
   - `STBY`
   - UART TX/RX
   - ultrasonic `TRIG`/`ECHO`
2. Implement real `Motor_SetCommand()` using PWM + GPIO.
3. Replace `Wireless_MockInput()` with UART packet parsing.
4. Replace `Sensor_MockRead()` with ultrasonic timing code.
