#include "joystick.h"

// Constructor delegates to Button for the push pin
Joystick::Joystick(uint8_t pinX, uint8_t pinY, uint8_t pinButton,
                   uint32_t debounceTicks,
                   uint32_t longPressTicks,
                   uint32_t doubleClickTicks,
                   ButtonPull pull)
    : Button(pinButton, debounceTicks, longPressTicks, doubleClickTicks, pull),
      _pinX(pinX), _pinY(pinY),
      _portX(0), _portY(0),
      _baseX(0), _baseY(0),
      _maskX(0), _maskY(0),
      _adcCtlX(NOT_ON_ADC), _adcCtlY(NOT_ON_ADC),
      _adcBase(ADC0_BASE), _adcSeq(0), _adcInit(false),
      _minX(0), _centerX(2048), _maxX(4095),
      _minY(0), _centerY(2048), _maxY(4095),
      _rawX(0), _rawY(0),
      _fx(0), _fy(0),
      _nx(0), _ny(0),
      _mag(0), _angle(0),
      _dir(JoystickDir::Center), _lastDir(JoystickDir::Center),
      _invertX(false), _invertY(false),
      _deadzone(0.10f), _alpha(0.20f), _moveEps(0.02f),
      _dirMagMinUp(0.30f), _dirMagMinDown(0.22f),
      _repeatMs(150), _tickMs(10),
      _nowMs(0), _lastRepeatMs(0),
      _lastNx(0), _lastNy(0), _tiltActive(false)
{
}

void Joystick::begin() {
    // Map pins → port/base/mask and ADC channels
    configureGpioAnalog(_pinX, _portX, _baseX, _maskX);
    configureGpioAnalog(_pinY, _portY, _baseY, _maskY);

    _adcCtlX = digital_pin_to_analog_in[_pinX];
    _adcCtlY = digital_pin_to_analog_in[_pinY];

    // Basic validity (channels must be ADC_CTL_CHx)
    if (_adcCtlX == NOT_ON_ADC || _adcCtlY == NOT_ON_ADC) {
        // Keep defaults; you may assert/log here in your environment
    }

    // Enable ADC0 and configure clock similar to example if desired
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) {}

    // Optional: if you use PLL-based ADC clock, configure here by your project convention
    // The simple path: use default clock

    configureAdcSequencer();
    _adcInit = true;

    // Initialize Button (push)
    Button::begin();
}

void Joystick::tick() {
    // Advance software timebase
    _nowMs += _tickMs;

    if (_adcInit) {
        readAdc2(_rawX, _rawY);
        filterAndNormalize();
        computePolarAndEvents();
    }

    // Tick the push-button FSM (reads physical via Button logic)
    Button::tick();
}

void Joystick::sanitizeRanges() {
    if (_minX > _maxX) { uint16_t t = _minX; _minX = _maxX; _maxX = t; }
    if (_minY > _maxY) { uint16_t t = _minY; _minY = _maxY; _maxY = t; }
    if (!(_minX <= _centerX && _centerX <= _maxX)) _centerX = (_minX + _maxX) / 2;
    if (!(_minY <= _centerY && _centerY <= _maxY)) _centerY = (_minY + _maxY) / 2;
}

void Joystick::configureGpioAnalog(uint8_t pin, uint8_t& port, uint32_t& base, uint8_t& mask) {
    port = digital_pin_to_port[pin];
    base = port_to_base[port];
    mask = digital_pin_to_bit_mask[pin];

    uint32_t periph = sysctl_periph_for_port(port);
    if (periph) {
        SysCtlPeripheralEnable(periph);
        while(!SysCtlPeripheralReady(periph)) {}
        GPIOPinTypeADC(base, mask);
    }
}

void Joystick::configureAdcSequencer() {
    // Use ADC0, sequence 0 (two steps: X then Y)
    ADCSequenceDisable(_adcBase, _adcSeq);
    ADCSequenceConfigure(_adcBase, _adcSeq, ADC_TRIGGER_PROCESSOR, 0);

    // Optionally swap assignment if hardware axes are inverted
    uint32_t step0 = _swapXY ? _adcCtlY : _adcCtlX;
    uint32_t step1 = _swapXY ? _adcCtlX : _adcCtlY;

    // Step 0: first axis
    ADCSequenceStepConfigure(_adcBase, _adcSeq, 0, step0);
    // Step 1: second axis + interrupt + end
    ADCSequenceStepConfigure(_adcBase, _adcSeq, 1, step1 | ADC_CTL_IE | ADC_CTL_END);

    ADCSequenceEnable(_adcBase, _adcSeq);
}

void Joystick::readAdc2(uint16_t& x, uint16_t& y) {
    uint32_t tmp[2] = {0};
    ADCProcessorTrigger(_adcBase, _adcSeq);
    while(!ADCIntStatus(_adcBase, _adcSeq, false)) {}
    ADCSequenceDataGet(_adcBase, _adcSeq, tmp);
    ADCIntClear(_adcBase, _adcSeq);
    uint16_t a0 = (uint16_t)(tmp[0] & 0xFFF);
    uint16_t a1 = (uint16_t)(tmp[1] & 0xFFF);
    if (_swapXY) { x = a1; y = a0; } else { x = a0; y = a1; }
}

void Joystick::filterAndNormalize() {
    // Convert raw → signed normalized around center, per-axis, then IIR
    auto normAxis = [](uint16_t raw, uint16_t minV, uint16_t centerV, uint16_t maxV) -> float {
        if (raw >= centerV) {
            uint16_t span = (maxV > centerV) ? (maxV - centerV) : 1;
            float v = (float)(raw - centerV) / (float)span; // [0..1]
            return v; // positive
        } else {
            uint16_t span = (centerV > minV) ? (centerV - minV) : 1;
            float v = (float)(centerV - raw) / (float)span; // [0..1]
            return -v; // negative
        }
    };

    float tx = normAxis(_rawX, _minX, _centerX, _maxX);
    float ty = normAxis(_rawY, _minY, _centerY, _maxY);

    if (_invertX) tx = -tx;
    if (_invertY) ty = -ty;

    // IIR smoothing
    _fx = _alpha * tx + (1.0f - _alpha) * _fx;
    _fy = _alpha * ty + (1.0f - _alpha) * _fy;

    // Radial deadzone: shrink circle of radius deadzone to 0; scale the rest to [0..1]
    float r = sqrtf(_fx * _fx + _fy * _fy);
    float nx = 0.0f, ny = 0.0f;
    if (r <= _deadzone) {
        nx = 0.0f; ny = 0.0f;
    } else {
        float k = (r - _deadzone) / (1.0f - _deadzone); // [0..1]
        k = (k < 0.0f ? 0.0f : (k > 1.0f ? 1.0f : k));
        float scale = (r > 0.0f) ? (k / r) : 0.0f;
        nx = _fx * scale;
        ny = _fy * scale;
    }

    _nx = nx;
    _ny = ny;
}

static inline float angle_wrap(float a) {
    const float PI = 3.14159265358979323846f;
    while (a <= -PI) a += 2*PI;
    while (a > PI) a -= 2*PI;
    return a;
}

JoystickDir Joystick::quantize8(float ang) const {
    // Map angle to sectors centered at N(90°), E(0°), etc. Use atan2(y,x) where 0 = E
    const float PI = 3.14159265358979323846f;
    const float step = (float)PI / 4.0f; // 45°
    // shift so that E is centered at 0 → sectors: E(0), NE(45), N(90), ...
    int idx = (int)floorf((ang + step/2.0f) / step);
    // idx in [-4..+4), wrap to [0..7]
    int s = idx & 7;
    switch (s) {
        case 0: return JoystickDir::E;
        case 1: return JoystickDir::NE;
        case 2: return JoystickDir::N;
        case 3: return JoystickDir::NW;
        case 4: return JoystickDir::W;
        case 5: return JoystickDir::SW;
        case 6: return JoystickDir::S;
        case 7: return JoystickDir::SE;
        default: return JoystickDir::Center;
    }
}

void Joystick::computePolarAndEvents() {
    // Polar values
    _mag = sqrtf(_nx * _nx + _ny * _ny);
    _angle = atan2f(_ny, _nx); // radians, 0 = +X (east)

    // Direction with magnitude hysteresis
    JoystickDir newDir = _dir;
    if (_mag >= _dirMagMinUp) {
        newDir = quantize8(_angle);
    } else if (_mag <= _dirMagMinDown) {
        newDir = JoystickDir::Center;
    }

    bool tiltNow = (_mag >= _dirMagMinUp);

    // Events: tilt start/stop
    if (tiltNow && !_tiltActive) {
        emitTiltStart();
        _tiltActive = true;
    } else if (!tiltNow && _tiltActive) {
        emitTiltStop();
        emitCenter();
        _tiltActive = false;
    }

    // Direction changed
    if (newDir != _dir) {
        _lastDir = _dir;
        _dir = newDir;
        emitDirChanged();
        _lastRepeatMs = _nowMs; // reset repeat timer on change
    }

    // Move event (epsilon on nx, ny)
    if (fabsf_fast(_nx - _lastNx) >= _moveEps || fabsf_fast(_ny - _lastNy) >= _moveEps) {
        emitMove();
        _lastNx = _nx; _lastNy = _ny;
    }

    // Repeat while a direction is active
    if (_dir != JoystickDir::Center && (_nowMs - _lastRepeatMs) >= _repeatMs) {
        emitRepeat();
        _lastRepeatMs = _nowMs;
    }
}

void Joystick::calibrateCenter(uint16_t samples) {
    // Simple blocking center calibration; assumes stick is at rest
    uint32_t accX = 0, accY = 0;
    for (uint16_t i = 0; i < samples; ++i) {
        uint16_t rx, ry; readAdc2(rx, ry);
        accX += rx; accY += ry;
    }
    _centerX = (uint16_t)(accX / samples);
    _centerY = (uint16_t)(accY / samples);
}

// Emit helpers: prefer simplified Events (void(Joystick&)) then fallback to legacy attach API
inline void Joystick::emitMove() {
    if (_events.move) { _events.move(*this); return; }
    if (_onMove) _onMove();
    if (_onMoveP) _onMoveP(_onMoveParam);
}

inline void Joystick::emitTiltStart() {
    if (_events.tiltStart) { _events.tiltStart(*this); return; }
    if (_onTiltStart) _onTiltStart();
    if (_onTiltStartP) _onTiltStartP(_onTiltStartParam);
}

inline void Joystick::emitTiltStop() {
    if (_events.tiltStop) { _events.tiltStop(*this); return; }
    if (_onTiltStop) _onTiltStop();
    if (_onTiltStopP) _onTiltStopP(_onTiltStopParam);
}

inline void Joystick::emitDirChanged() {
    if (_events.directionChanged) { _events.directionChanged(*this); return; }
    if (_onDirChanged) _onDirChanged();
    if (_onDirChangedP) _onDirChangedP(_onDirChangedParam);
}

inline void Joystick::emitRepeat() {
    if (_events.repeat) { _events.repeat(*this); return; }
    if (_onRepeat) _onRepeat();
    if (_onRepeatP) _onRepeatP(_onRepeatParam);
}

inline void Joystick::emitCenter() {
    if (_events.center) { _events.center(*this); return; }
    if (_onCenter) _onCenter();
    if (_onCenterP) _onCenterP(_onCenterParam);
}
