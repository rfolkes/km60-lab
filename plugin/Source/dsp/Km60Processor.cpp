#include "Km60Processor.h"
#include <cmath>

// Shelf frequencies derived from the KM-60 schematic (AP-90 board).
// Treble: 1/(2π × 8.2kΩ × 12nF) = 1617 Hz, rounded to 1600.
// Bass:   estimated ~350 Hz from cap/resistor values — refine after measurement.
static constexpr double kTrebleHz = 1600.0;
static constexpr double kBassHz   = 350.0;
static constexpr double kPi       = 3.141592653589793;

// ─── Biquad helpers ──────────────────────────────────────────────────────────

// Direct Form II Transposed — better numerical behaviour than standard DF2.
float Km60Processor::applyBiquad(float x, const BiquadCoeffs& c, BiquadState& s) noexcept
{
    const float y = c.b0 * x + s.w1;
    s.w1 = c.b1 * x - c.a1 * y + s.w2;
    s.w2 = c.b2 * x - c.a2 * y;
    return y;
}

// Audio EQ Cookbook shelf formulas (R. Bristow-Johnson), Q = 1/√2 (Butterworth slope).
Km60Processor::BiquadCoeffs
Km60Processor::makeHighShelf(double fc, double gainDb, double sampleRate)
{
    const double A     = std::pow(10.0, gainDb / 40.0);
    const double w0    = 2.0 * kPi * fc / sampleRate;
    const double cosw0 = std::cos(w0);
    const double sinw0 = std::sin(w0);
    const double sqrtA = std::sqrt(A);
    // Q = 1/√2  →  alpha = sin(w0) / (2Q) = sin(w0) × √2 / 2
    const double alpha = sinw0 * std::sqrt(2.0) * 0.5;

    const double b0 =    A * ((A+1) + (A-1)*cosw0 + 2.0*sqrtA*alpha);
    const double b1 = -2.0*A * ((A-1) + (A+1)*cosw0);
    const double b2 =    A * ((A+1) + (A-1)*cosw0 - 2.0*sqrtA*alpha);
    const double a0 =        (A+1) - (A-1)*cosw0 + 2.0*sqrtA*alpha;
    const double a1 =  2.0 * ((A-1) - (A+1)*cosw0);
    const double a2 =        (A+1) - (A-1)*cosw0 - 2.0*sqrtA*alpha;

    return { (float)(b0/a0), (float)(b1/a0), (float)(b2/a0),
             (float)(a1/a0), (float)(a2/a0) };
}

Km60Processor::BiquadCoeffs
Km60Processor::makeLowShelf(double fc, double gainDb, double sampleRate)
{
    const double A     = std::pow(10.0, gainDb / 40.0);
    const double w0    = 2.0 * kPi * fc / sampleRate;
    const double cosw0 = std::cos(w0);
    const double sinw0 = std::sin(w0);
    const double sqrtA = std::sqrt(A);
    const double alpha = sinw0 * std::sqrt(2.0) * 0.5;

    const double b0 =    A * ((A+1) - (A-1)*cosw0 + 2.0*sqrtA*alpha);
    const double b1 =  2.0*A * ((A-1) - (A+1)*cosw0);
    const double b2 =    A * ((A+1) - (A-1)*cosw0 - 2.0*sqrtA*alpha);
    const double a0 =        (A+1) + (A-1)*cosw0 + 2.0*sqrtA*alpha;
    const double a1 = -2.0 * ((A-1) + (A+1)*cosw0);
    const double a2 =        (A+1) + (A-1)*cosw0 - 2.0*sqrtA*alpha;

    return { (float)(b0/a0), (float)(b1/a0), (float)(b2/a0),
             (float)(a1/a0), (float)(a2/a0) };
}

// ─── Public interface ─────────────────────────────────────────────────────────

void Km60Processor::prepare(double sr)
{
    sampleRate = sr;
    reset();
}

void Km60Processor::reset()
{
    trebleState[0].reset();
    trebleState[1].reset();
    bassState[0].reset();
    bassState[1].reset();
}

void Km60Processor::updateCoefficients(float trebleDb, float bassDb)
{
    trebleCoeffs = makeHighShelf(kTrebleHz, trebleDb, sampleRate);
    bassCoeffs   = makeLowShelf (kBassHz,   bassDb,   sampleRate);
}

// HA-1457 saturation model.
// Linear below the clipping threshold (x <= 1.0), then a slope-matched soft knee above.
// The threshold is normalised to 1.0: saturation only occurs when inputGain pushes the
// signal above that level, exactly as overloading the real preamp input would.
// C¹ continuity at the knee is guaranteed because (ceiling × k) = 1.0 matches the
// linear slope — avoids the click that a hard slope discontinuity would produce.
float Km60Processor::softClipHa1457(float x) noexcept
{
    const float absX = std::abs(x);
    if (absX <= 1.0f) return x;

    // ceiling=0.1, k=10 → slope at knee = 0.1 × 10 × sech²(0) = 1.0 ✓
    return std::copysign(1.0f + 0.1f * std::tanh((absX - 1.0f) * 10.0f), x);
}

float Km60Processor::processSample(float x, int channel,
                                    float inputGain, float outputGain) noexcept
{
    x *= inputGain;

    // Baxandall-style shelving EQ (schematic-derived frequencies),
    // before the saturation stage — matching the KM-60 signal chain.
    x = applyBiquad(x, trebleCoeffs, trebleState[channel]);
    x = applyBiquad(x, bassCoeffs,   bassState[channel]);

    x = softClipHa1457(x);

    x *= outputGain;
    return x;
}
