#include "hardware.h"

HardwareManager::HardwareManager() {
    // initialize buffer
    for (size_t i = 0; i < _bufferSize; ++i) _buffer[i] = 0;
    pinMode(indicatorLEDPin, OUTPUT);
}

void HardwareManager::begin() {
    // Configure ADC for 12-bit and full-scale (approx 0-3.3V)
    // analogSetWidth and analogSetPinAttenuation are available in the ESP32 Arduino core
    analogSetWidth(12);
    analogSetPinAttenuation(_adcPin, ADC_11db); // allow up to ~3.6V input range

    _lastSampleUs = micros();
}

void HardwareManager::update() {
    // Called repeatedly from the hardware FreeRTOS task; sample at the configured rate
    uint32_t now = micros();
    if ((now - _lastSampleUs) >= _sampleIntervalUs) {
        // Sample ADC
        uint16_t raw = analogRead(_adcPin);

        // store in buffer
        _buffer[_bufferHead] = raw;
        _bufferHead++;
        if (_bufferHead >= _bufferSize) _bufferHead = 0;

        // Convert to voltage (approx)
        _latestVoltage = (float)raw * (3.3f / (float)_adcMax);

        _lastSampleUs = now;
        // Analyze buffer for 440Hz signal every full buffer
        if (_bufferHead == 0) {
            analyzeSignal();
        } 
        
    }
}

// Returns 1 if a 440Hz signal is detected, 0 if not
int HardwareManager::getSignalLockState() {
    return _signalLockState;
}

// Helper: analyze buffer for 440Hz presence
void HardwareManager::analyzeSignal() {
    // Simple zero-crossing frequency detection
    // Assumes buffer is filled at 4kHz, so 1024 samples = 0.256s
    // 440Hz should have about 300-500 cycles in 1024 samples
    // We'll count zero crossings i.e. cycles
    // Calculate RMS of buffer
    float rms = getBufferRMS();

    // Use RMS as the zero-crossing threshold
    int zeroCrossings = 0;
    float prev = ((float)_buffer[0] * 3.3f / (float)_adcMax) - rms;
    for (size_t i = 1; i < _bufferSize; ++i) {
        float v = ((float)_buffer[i] * 3.3f / (float)_adcMax) - rms;
        if ((prev < 0 && v >= 0) || (prev > 0 && v <= 0)) {
            zeroCrossings++;
        }
        prev = v;
    }
    // Each cycle has 2 zero crossings, so cycles = zeroCrossings / 2
    float cycles = zeroCrossings / 2.0f;
    Serial.print("analyzeSignal: cycles = ");
    Serial.println(cycles, 2);
    // 440Hz * 0.256s = ~112.6 cycles
    // Accept all cycles not equal to zero
    if (cycles != 0) {
        _signalLockState = 1;
    } else {
        _signalLockState = 0;
    }
}

float HardwareManager::getLatestVoltage() {
    return _latestVoltage;
}

float HardwareManager::getBufferRMS() {
    // Compute RMS over the buffer. This is not fully lock-protected but
    // acceptable for small reads in this simple application.
    double sumSq = 0.0;
    for (size_t i = 0; i < _bufferSize; ++i) {
        double v = (double)_buffer[i] * (3.3 / (double)_adcMax);
        sumSq += v * v;
    }
    double meanSq = sumSq / (double)_bufferSize;
    return (float)sqrt(meanSq);
}




// Print signal stats every second
        /*
        static uint32_t lastPrintMs = 0;
        uint32_t nowMs = millis();
        if (nowMs - lastPrintMs > 1000) {
            int lock = getSignalLockState();
            // Frequency estimation from zero crossings
            int zeroCrossings = 0;
            float prev = ((float)_buffer[0] * 3.3f / (float)_adcMax) - 1.65f;
            for (size_t i = 1; i < _bufferSize; ++i) {
                float v = ((float)_buffer[i] * 3.3f / (float)_adcMax) - 1.65f;
                if ((prev < 0 && v >= 0) || (prev > 0 && v <= 0)) {
                    zeroCrossings++;
                }
                prev = v;
            }
            float cycles = zeroCrossings / 2.0f;
            float freq = cycles / (_bufferSize * _sampleIntervalUs * 1e-6f); // cycles per buffer time
            Serial.print("V, lock state: ");
            Serial.println(lock == 1 ? "LOCKED" : "UNLOCKED");
            Serial.print(", freq: ");
            Serial.print(freq, 1);
            Serial.println(" Hz");
            lastPrintMs = nowMs;
        }
            */