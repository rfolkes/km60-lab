#pragma once

// Stateful DSP for the KM60Lab channel strip.
// Contains the measurement-derived Baxandall EQ and the HA-1457 saturation model.
class Km60Processor
{
public:
    void prepare(double sampleRate);
    void reset();

    // Recompute EQ biquad coefficients from new gain values.
    // Call once per block before the sample loop — trig-heavy, not per-sample.
    void updateCoefficients(float trebleDb, float bassDb);

    // Process one sample. channel must be 0 (L) or 1 (R).
    // inputGain and outputGain are linear gains.
    float processSample(float x, int channel,
                        float inputGain, float outputGain) noexcept;

private:
    struct BiquadCoeffs
    {
        float b0 = 1.f, b1 = 0.f, b2 = 0.f;  // feedforward
        float a1 = 0.f, a2 = 0.f;              // feedback (normalised, a0 = 1)
    };

    // Direct Form II Transposed state — two words per filter per channel.
    struct BiquadState
    {
        float w1 = 0.f, w2 = 0.f;
        void reset() noexcept { w1 = w2 = 0.f; }
    };

    static BiquadCoeffs makeHighShelf(double fc, double gainDb, double sampleRate);
    static BiquadCoeffs makeLowShelf (double fc, double gainDb, double sampleRate);
    static float applyBiquad(float x, const BiquadCoeffs& c, BiquadState& s) noexcept;

    // Saturation model based on the HA-1457's measured characteristic.
    static float softClipHa1457(float x) noexcept;

    double sampleRate = 44100.0;

    BiquadCoeffs trebleCoeffs;
    BiquadCoeffs bassCoeffs;
    BiquadState  trebleState[2];  // [L, R]
    BiquadState  bassState[2];
};
