#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>

// Hardware manager: handles on-board peripherals including ADC sampling.
class HardwareManager {
public:
    HardwareManager();
    void begin();
    void update();

    // Access latest ADC measurements
    float getLatestVoltage();
    float getBufferRMS();

    // Returns 1 if a 440Hz signal is detected, 0 if not (IR off lock)
    int getSignalLockState();

private:
    // ADC config
    const int _adcPin = 34; // ADC1 channel (input-only) on many ESP32 boards
    const int indicatorLEDPin = 14; // LED indicator pin 
    const uint16_t _adcMax = 4095; // 12-bit ADC on ESP32

    // Sampling buffer (circular)
    static const size_t _bufferSize = 1024;
    uint16_t _buffer[_bufferSize];
    size_t _bufferHead = 0;

    // Sampling timing
    uint32_t _sampleIntervalUs = 250; // default 4 kHz sample rate => 250 us
    uint32_t _lastSampleUs = 0;

    // Latest converted voltage cached
    volatile float _latestVoltage = 0.0f;

    // Logic state: 1 if 440Hz detected, 0 if not
    volatile int _signalLockState = 0;

    // Helper to analyze buffer for 440Hz
    void analyzeSignal();
};

#endif 
