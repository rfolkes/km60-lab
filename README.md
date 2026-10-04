# KM60Lab

A native C++ audio plugin modelled on the Boss KM-60 compact mixer (1978). Built with JUCE 8 and CMake.

**Current state:** working VST3 — stereo in/out, measurement-informed Baxandall EQ, HA-1457 saturation model.

---

## Prerequisites

### macOS

- macOS 10.13 or later
- Xcode (command-line tools are enough for a CLI build, full Xcode needed for IDE)
- CMake 3.22 or later
- Git

```bash
cmake --version          # verify
brew install cmake       # if missing
```

### Windows (64-bit VST3)

- Windows 10 or later (64-bit)
- [Build Tools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022) (or later) — select the **"Desktop development with C++"** workload during install
- CMake 3.22 or later (bundled with VS Build Tools, or install standalone from [cmake.org](https://cmake.org/download/))
- Git

The full Visual Studio IDE is not required — the Build Tools package is sufficient.

---

## Build

The first configure step downloads JUCE (~100 MB shallow clone).

### macOS

```bash
# Debug build (faster to compile, better for development)
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(sysctl -n hw.logicalcpu)

# Release build
cmake -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j$(sysctl -n hw.logicalcpu)
```

### Windows

Open a **Developer PowerShell for VS** (installed with the Build Tools) and run:

```powershell
# Configure (64-bit)
cmake -B build -G "Visual Studio 17 2022" -A x64

# Debug build
cmake --build build --config Debug

# Release build
cmake --build build --config Release
```

If you prefer faster builds with Ninja (also included with the Build Tools):

```powershell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

---

## Where the VST3 ends up

`COPY_PLUGIN_AFTER_BUILD TRUE` is set in [plugin/CMakeLists.txt](plugin/CMakeLists.txt), so after a
successful build JUCE copies the plugin automatically to the system VST3 folder.

### macOS

```
~/Library/Audio/Plug-Ins/VST3/KM60Lab.vst3
```

The intermediate build artefact lives at:

```
build/plugin/KM60Lab_artefacts/Debug/VST3/KM60Lab.vst3
```

To install manually if the auto-copy fails:
```bash
cp -r build/plugin/KM60Lab_artefacts/Debug/VST3/KM60Lab.vst3 \
      ~/Library/Audio/Plug-Ins/VST3/
```

### Windows

```
C:\Program Files\Common Files\VST3\KM60Lab.vst3
```

The intermediate build artefact lives at:

```
build\plugin\KM60Lab_artefacts\Release\VST3\KM60Lab.vst3
```

To install manually, copy the `.vst3` folder to the system path above (may require administrator privileges).

---

## Testing in Ableton Live

1. Close Ableton before rebuilding — macOS locks the bundle while it is loaded.
2. Build the plugin.
3. Open Ableton → Preferences → Plug-Ins → enable **VST3 System Folders**.
4. Click **Rescan** (or restart Ableton).
5. Find **KM60Lab** in the plugin browser and load it on an audio track.
6. Verify audio passes through cleanly at default settings.
7. Push **Input Gain** above +14 dB on a hot source to start hearing the preamp saturation;
   pull back with **Output Gain** to compensate level.

---

## Parameters

| Parameter   | Range          | Default | Notes                                                        |
|-------------|----------------|---------|--------------------------------------------------------------|
| Input Gain  | −24 to +36 dB  | 0 dB    | Extends into saturation territory — see below                |
| Treble      | −15 to +15 dB  | 0 dB    | High shelf at 8 kHz (measured from hardware unit)            |
| Bass        | −15 to +15 dB  | 0 dB    | Low shelf at 81 Hz (measured from hardware unit)             |
| Output Gain | −24 to +24 dB  | 0 dB    | Compensate level after saturation                            |

Input Gain and Output Gain are smoothed with a 20 ms ramp to prevent zipper noise.
EQ coefficients update once per block (sufficient for non-automated shelf controls).

---

## Saturation model

The KM-60 has no drive knob. Saturation in the real unit occurs when the input signal
overloads the HA-1457 op-amp preamp stage on each channel strip.

The HA-1457 (Hitachi, ±14V supply in the KM-60) clips at ~9.2V peak. With a nominal
channel level of +4 dBm = 1.74V peak, there is roughly **14 dB of clean headroom**
before saturation becomes audible. The datasheet confirms THD below 0.005% at 5V output
— the chip is extremely clean in normal use.

In the plugin this is modelled as a **slope-matched soft-knee clipper** that is
mathematically transparent below the threshold and transitions smoothly above it:

```
x <= 1.0  →  y = x                         (clean — linear region)
x >  1.0  →  y = 1.0 + 0.1 × tanh((x−1) × 10)   (soft knee, ceiling ≈ +0.83 dBFS)
```

The slope is continuous at the knee (derivative = 1 on both sides), avoiding transient
artefacts when a loud signal crosses the threshold.

**How to use:** at 0 dB Input Gain a typical −18 dBFS source has ~18 dB of margin before
reaching the knee. Push Input Gain above that to introduce saturation; use Output Gain to
bring the level back down — the same workflow as overloading a real channel strip.

> The exact knee shape and harmonic content (H2/H3 ratio from the HA-1457's push-pull
> output stage) will be refined with THD+N and harmonic profile measurements (Session 2).

---

## EQ

The Treble and Bass shelves are measured from the real KM-60 hardware unit (Session 1, Sep 2026)
using REW swept-sine captures. Turnover frequency = −3 dB below the shelf maximum,
passband-normalised relative to a flat reference capture.

**Treble — 8 kHz high shelf, ±15 dB**  
A proper "air" shelf. Much higher than the ~1600 Hz figure estimated from the schematic RC values
— the Baxandall topology places its −3 dB turnover well above the component resonance frequency.

**Bass — 81 Hz low shelf, ±15 dB**  
A sub-bass shelf that primarily affects kick drum fundamentals and bass guitar fundamentals.
Cuts off quickly above 200 Hz, leaving the upper bass and mids untouched.

---

## JUCE version

Fetched via CMake `FetchContent`. Tag is set in [CMakeLists.txt](CMakeLists.txt).
Check the JUCE GitHub releases page for the latest 8.x tag and update `GIT_TAG` if needed.

---

## Project layout

```
plugin/
  CMakeLists.txt
  Source/
    PluginProcessor.h / .cpp   — JUCE AudioProcessor, APVTS, parameter smoothing
    PluginEditor.h / .cpp      — Generic JUCE editor (placeholder for custom UI)
    dsp/
      Km60Processor.h / .cpp   — All DSP: Baxandall EQ + HA-1457 saturation model
measurements/
  captures/                    — Raw measurement files (REW exports, WAV captures)
  scripts/                     — Python analysis scripts
  analysis/                    — Processed results, plots
docs/
  hardware-notes.md            — KM-60 schematic analysis, HA-1457 datasheet findings
  measurement-plan.md          — Capture plan; Session 1 (EQ sweeps) complete
test-audio/                    — Short clips for manual A/B testing
```
