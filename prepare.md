# Super-RC Prepare Guide

## What To Do Next

### Right now, before parts arrive

1. Keep the project compiling cleanly.
2. Test the mock wireless + mock sensor behavior.
3. Watch debug output and confirm the control flow is correct.
4. Record any wrong behavior in a simple bug log.
5. Fix logic problems now, before hardware is added.

### When parts arrive

1. Set up power safely first.
2. Pick and write down exact TM4C pins.
3. Wire one subsystem at a time.
4. Replace mocks with real hardware drivers one file at a time.
5. Test each subsystem alone before combining everything.

## Current Software Flow

The current intended flow is:

1. `WirelessRxTask` updates command data
2. `SensorTask` updates obstacle data
3. `SafetyMonitorTask` checks timeout and faults
4. `ControlTask` computes final motion
5. `MotorTask` executes final motion
6. `DebugTask` prints state and safety changes

## What You Should Test Now

### Main cases

Run the current project and verify these cases from debug output:

1. forward command happens
2. stop command happens
3. left command happens
4. right command happens
5. obstacle during forward causes stop
6. auto mode obstacle causes avoid behavior
7. signal timeout causes safe stop

### What to watch in debug output

Check:

- received command
- current mode
- obstacle detected or not
- final motion
- motor output
- timeout flag
- safety override event

## How To Record Bugs

Use this format:

```text
Test:
Expected:
Actual:
Debug Output:
Possible Cause:
```

### Example

```text
Test: obstacle during forward in manual mode
Expected: final motion should become STOP
Actual: motor kept moving forward for one cycle
Debug Output: CMD rx=FWD ... obs=1 final=FWD ...
Possible Cause: control task ran before updated sensor state was used
```

## What To Report Back

When you test, send back:

1. which test case you ran
2. what you expected
3. what actually happened
4. the relevant debug lines
5. whether it happens every time or only sometimes

That is enough to debug the issue quickly.

## Setup When Parts Arrive

### Step 1: Safe bench setup

- power the TM4C1294XL by USB first
- do not connect motor power yet
- keep a shared ground plan ready
- keep the wheels off the ground during first motor tests

### Step 2: Pick pins

Write these down before wiring:

- TB6612FNG:
  - `PWMA`
  - `PWMB`
  - `AIN1`
  - `AIN2`
  - `BIN1`
  - `BIN2`
  - `STBY`
- HC-05:
  - UART TX
  - UART RX
- HC-SR04:
  - `TRIG`
  - `ECHO`

### Step 3: Bring up hardware in this order

1. debug output
2. HC-05 UART
3. motor driver GPIO
4. motor PWM
5. ultrasonic sensor

Do not wire everything at once.

## What Changes Later

### `motor.cpp`

Replace stub logic with:

- PWM setup
- direction GPIO
- TB6612FNG command mapping

### `wireless.cpp`

Replace mock logic with:

- UART init
- packet receive
- parse commands like `<F,60,0>`

### `sensor.cpp`

Replace mock logic with:

- trigger pulse
- echo timing
- distance calculation

## Packet Format To Keep

Use:

```text
<F,60,0>
<L,40,0>
<S,0,0>
```

Meaning:

- first field = motion
- second field = speed percent
- third field = mode

Examples:

- `F,60,0` = forward, 60%, manual
- `R,50,0` = reverse, 50%, manual
- `S,0,0` = stop
- `F,60,1` = forward, 60%, auto mode

## Most Important Goal Right Now

Do not try to finish hardware code early.

The main goal right now is:

1. test the existing software behavior
2. find bugs in timing or logic
3. record what happened
4. report it back
5. fix the software before hardware integration starts
