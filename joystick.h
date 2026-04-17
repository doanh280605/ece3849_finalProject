#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

#include "pins.h"      // pin → port/base/mask/ADC channel mapping
#include "button.h"    // inherits for stick push handling

// Direction encoding for 8-way joystick
enum class JoystickDir : uint8_t {
    Center = 0,
    N, NE, E, SE, S, SW, W, NW
};

class Joystick : public Button {
public:
    // Callback types (mirroring Button style)
    typedef void (*callbackFunction)(void);
    typedef void (*parameterizedCallbackFunction)(void*);

    // Constructor: pins are Energia-style IDs from pins.h (e.g., PD_2, PK_1)
    Joystick(uint8_t pinX,
             uint8_t pinY,
             uint8_t pinButton,
             uint32_t debounceTicks = 3,
             uint32_t longPressTicks = 50,
             uint32_t doubleClickTicks = 25,
             ButtonPull pull = ButtonPull::PullUp);

    // Lifecycle
    void begin();

    // Polling (call periodically at ~tickIntervalMs)
    void tick();

    // Raw readings (12-bit ADC)
    uint16_t rawX() const { return _rawX; }
    uint16_t rawY() const { return _rawY; }

    // Normalized values in [-1, 1] after filtering and deadzone mapping
    float x() const { return _nx; }
    float y() const { return _ny; }

    // Polar
    float magnitude() const { return _mag; }   // [0..1]
    float angleRad() const { return _angle; }  // radians
    float angleDeg() const { return _angle * 180.0f / 3.14159265358979323846f; }

    // Direction (8-way)
    JoystickDir direction8() const { return _dir; }

    // Configuration
    void setDeadzone(float dz) { _deadzone = clamp01(dz); }
    void setSmoothingAlpha(float a) { _alpha = clamp01(a); }
    void setInvertX(bool inv) { _invertX = inv; }
    void setInvertY(bool inv) { _invertY = inv; }

    void setRangeX(uint16_t min, uint16_t center, uint16_t max) { _minX = min; _centerX = center; _maxX = max; sanitizeRanges(); }
    void setRangeY(uint16_t min, uint16_t center, uint16_t max) { _minY = min; _centerY = center; _maxY = max; sanitizeRanges(); }

    void calibrateCenter(uint16_t samples = 32);

    void setDirectionThreshold(float magMin) { _dirMagMinUp = clamp01(magMin); if (_dirMagMinDown > _dirMagMinUp) _dirMagMinDown = _dirMagMinUp; }
    void setDirectionHysteresis(float magBack, float /*degBack*/) { _dirMagMinDown = clamp01(magBack); }

    void setMoveEpsilon(float eps) { _moveEps = (eps < 0.0f ? 0.0f : eps); }
    void setRepeatIntervalMs(uint32_t ms) { _repeatMs = ms; }
    void setTickIntervalMs(uint32_t ms) { _tickMs = (ms == 0 ? 1u : ms); Button::setTickIntervalMs(_tickMs); }

    // Simplified event API: register all at once
    struct Events {
        void (*move)(Joystick&) = nullptr;
        void (*tiltStart)(Joystick&) = nullptr;
        void (*tiltStop)(Joystick&) = nullptr;
        void (*directionChanged)(Joystick&) = nullptr;
        void (*repeat)(Joystick&) = nullptr;
        void (*center)(Joystick&) = nullptr;
    };

    void setEvents(const Events& e) { _events = e; }

    // Chainable helpers (fluent style)
    Joystick& onMove(void (*fn)(Joystick&)) { _events.move = fn; return *this; }
    Joystick& onTiltStart(void (*fn)(Joystick&)) { _events.tiltStart = fn; return *this; }
    Joystick& onTiltStop(void (*fn)(Joystick&)) { _events.tiltStop = fn; return *this; }
    Joystick& onDirectionChanged(void (*fn)(Joystick&)) { _events.directionChanged = fn; return *this; }
    Joystick& onRepeat(void (*fn)(Joystick&)) { _events.repeat = fn; return *this; }
    Joystick& onCenter(void (*fn)(Joystick&)) { _events.center = fn; return *this; }

    // Legacy-style registration (kept for compatibility)
    // Prefer the simplified API above when possible.
    // Event registration
    void attachMove(callbackFunction fn) { _onMove = fn; }
    void attachMove(parameterizedCallbackFunction fn, void* p) { _onMoveP = fn; _onMoveParam = p; }

    void attachTiltStart(callbackFunction fn) { _onTiltStart = fn; }
    void attachTiltStart(parameterizedCallbackFunction fn, void* p) { _onTiltStartP = fn; _onTiltStartParam = p; }

    void attachTiltStop(callbackFunction fn) { _onTiltStop = fn; }
    void attachTiltStop(parameterizedCallbackFunction fn, void* p) { _onTiltStopP = fn; _onTiltStopParam = p; }

    void attachDirectionChanged(callbackFunction fn) { _onDirChanged = fn; }
    void attachDirectionChanged(parameterizedCallbackFunction fn, void* p) { _onDirChangedP = fn; _onDirChangedParam = p; }

    void attachRepeat(callbackFunction fn) { _onRepeat = fn; }
    void attachRepeat(parameterizedCallbackFunction fn, void* p) { _onRepeatP = fn; _onRepeatParam = p; }

    void attachCenter(callbackFunction fn) { _onCenter = fn; }
    void attachCenter(parameterizedCallbackFunction fn, void* p) { _onCenterP = fn; _onCenterParam = p; }

private:
    // Pin/port/channel mapping
    uint8_t  _pinX, _pinY;
    uint8_t  _portX, _portY;
    uint32_t _baseX, _baseY;
    uint8_t  _maskX, _maskY;
    uint32_t _adcCtlX, _adcCtlY; // ADC_CTL_CHxx constants

    // ADC configuration (ADC0, SS0 by default)
    uint32_t _adcBase;
    uint8_t  _adcSeq;
    bool     _adcInit;

    // Calibration ranges
    uint16_t _minX, _centerX, _maxX;
    uint16_t _minY, _centerY, _maxY;

    // Raw/filtered/normalized
    uint16_t _rawX, _rawY;
    float    _fx, _fy;   // filtered normalized per-axis (pre-deadzone)
    float    _nx, _ny;   // post-deadzone, scaled to [-1, 1]

    // Polar
    float    _mag;
    float    _angle;
    JoystickDir _dir;
    JoystickDir _lastDir;

    // Config flags
    bool _invertX, _invertY;

    // Tunables
    float    _deadzone;        // radial deadzone [0..1]
    float    _alpha;           // IIR smoothing factor [0..1]
    float    _moveEps;         // minimum delta to fire move
    float    _dirMagMinUp;     // require >= this to enter a direction
    float    _dirMagMinDown;   // drop below this to return to center
    uint32_t _repeatMs;        // repeat event while non-center
    uint32_t _tickMs;          // tick period (software timebase)

    // Timekeeping
    uint32_t _nowMs;
    uint32_t _lastRepeatMs;

    // Last reported for move epsilon
    float _lastNx, _lastNy;
    bool  _tiltActive;
    bool  _swapXY = false;     // if true, swap axis assignment (diagnostic/hw variant)

    // Events
    callbackFunction _onMove = nullptr;
    parameterizedCallbackFunction _onMoveP = nullptr; void* _onMoveParam = nullptr;

    callbackFunction _onTiltStart = nullptr;
    parameterizedCallbackFunction _onTiltStartP = nullptr; void* _onTiltStartParam = nullptr;

    callbackFunction _onTiltStop = nullptr;
    parameterizedCallbackFunction _onTiltStopP = nullptr; void* _onTiltStopParam = nullptr;

    callbackFunction _onDirChanged = nullptr;
    parameterizedCallbackFunction _onDirChangedP = nullptr; void* _onDirChangedParam = nullptr;

    callbackFunction _onRepeat = nullptr;
    parameterizedCallbackFunction _onRepeatP = nullptr; void* _onRepeatParam = nullptr;

    callbackFunction _onCenter = nullptr;
    parameterizedCallbackFunction _onCenterP = nullptr; void* _onCenterParam = nullptr;

    // Helpers
    static inline float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
    static inline float fabsf_fast(float v) { return v >= 0.0f ? v : -v; }

    void sanitizeRanges();
    void configureGpioAnalog(uint8_t pin, uint8_t& port, uint32_t& base, uint8_t& mask);
    void configureAdcSequencer();

    void readAdc2(uint16_t& x, uint16_t& y);
    
public:
    // Diagnostics/variants: swap XY assignment if board wiring differs
    void setSwapXY(bool en) { _swapXY = en; }
    void filterAndNormalize();
    void computePolarAndEvents();
    JoystickDir quantize8(float angleRad) const;
    inline void emitMove();
    inline void emitTiltStart();
    inline void emitTiltStop();
    inline void emitDirChanged();
    inline void emitRepeat();
    inline void emitCenter();

    // New event storage (preferred)
    Events _events;
};

#endif // JOYSTICK_H
