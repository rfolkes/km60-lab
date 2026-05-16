# CLAUDE.md

## Project

This project is `KM60Lab`: a native C++ audio plugin inspired by the Boss KM-60 compact mixer.

The current goal is not yet a perfect circuit-level emulation. The first goal is to build a stable, performant, cross-platform JUCE-based VST3 plugin that can later evolve into a measurement-informed model of the hardware.

Primary development environment:

- macOS on Intel
- VS Code
- Claude extension
- JUCE with CMake
- Ableton Live as the main test host
- VST3 first
- AU, Windows, installers and notarization later

## Developer background

The developer is experienced in C#, TypeScript and Python, but new to:

- C++
- JUCE
- VST3 plugin development
- real-time DSP
- hardware modelling

Prefer clear, idiomatic, maintainable C++ over clever code.

Explain important C++/DSP decisions briefly when making non-obvious changes.

## Technical direction

Use:

- JUCE
- CMake
- C++20
- VST3
- Native macOS build first

Avoid raw Steinberg VST3 SDK work unless explicitly needed.

The DSP should be kept separate from the JUCE plugin wrapper.

Preferred structure:

```text
plugin/
  Source/
    PluginProcessor.h
    PluginProcessor.cpp
    PluginEditor.h
    PluginEditor.cpp
    dsp/
      Km60Processor.h
      Km60Processor.cpp
      ChannelStrip.h
      ChannelStrip.cpp
measurements/
  captures/
  scripts/
  analysis/
docs/
  hardware-notes.md
  measurement-plan.md
test-audio/
```

## Real-time audio rules

The audio callback is sacred.

Inside `processBlock` and any code called from it:

- Do not allocate memory.
- Do not use locks or mutexes.
- Do not log.
- Do not access the filesystem.
- Do not call network APIs.
- Do not use exceptions for control flow.
- Do not perform slow GUI work.
- Do not change data structures that may allocate.
- Do not recalculate expensive coefficients per sample unless necessary.
- Do not use unbounded loops.
- Avoid virtual dispatch in the inner sample loop unless justified.

Use parameter smoothing for all user-facing continuous parameters.

Prepare all buffers, filters and coefficients in `prepareToPlay`.

Handle sample-rate changes correctly.

Handle variable block sizes correctly.

Handle mono/stereo host configurations defensively.

Avoid denormal problems in DSP code.

## Initial plugin milestone

Create a minimal working VST3 called `KM60Lab`.

Requirements:

- Stereo input
- Stereo output
- Generic JUCE editor is acceptable at first
- Parameters:
  - `inputGainDb`: -24 dB to +24 dB, default 0 dB
  - `drive`: 0 to 1, default 0
  - `outputGainDb`: -24 dB to +24 dB, default 0 dB

Initial DSP:

1. Apply input gain.
2. Apply simple soft saturation using a tanh-style waveshaper.
3. Apply output gain.
4. Preserve stereo.
5. Use smoothed parameter values.

No oversampling required in the first version.

Do not add a fancy UI yet.

## Testing

The first host target is Ableton Live.

The built VST3 should be installed or copied to:

```bash
~/Library/Audio/Plug-Ins/VST3/
```

Ableton setup:

- Enable VST3 system folders.
- Rescan plugins.
- Load `KM60Lab` on an audio track.
- Verify that audio passes.
- Verify that the gain and drive parameters work.
- Keep Ableton closed while rebuilding/copying the plugin.

Later, use pluginval for automated validation.

## DSP modelling direction{}

The long-term goal is a measurement-informed model of the Boss KM-60.

Prefer practical modelling before circuit-perfect modelling.

Likely model components:

- input gain/headroom
- input-stage saturation
- mild frequency coloration
- channel level
- panning
- send/return path
- summing bus coloration
- bus saturation
- output-stage saturation
- noise floor
- channel crosstalk
- possibly small channel-to-channel variation

Avoid jumping directly to neural modelling unless explicitly requested.

A good target is a musically useful “vintage compact line mixer” first, then increasingly informed by measurements of the real unit.

## Measurement phase, later

When hardware is available, measurements may include:

- clean frequency response
- frequency response at multiple gain settings
- THD+N versus level
- harmonic profile
- clipping behavior
- noise floor
- crosstalk between channels
- summing behavior with multiple active inputs
- send/return path behavior

Measurement data should be stored under:

```text
measurements/
  captures/
  analysis/
  scripts/
```

Use Python for measurement analysis where useful.

## Coding style

Prefer:

- small classes
- clear ownership
- RAII
- `std::array` for fixed-size channel structures
- `juce::AudioBuffer<float>` and JUCE DSP utilities where appropriate
- simple readable DSP code
- comments explaining real-time or DSP-sensitive decisions

Avoid:

- over-engineering
- large inheritance hierarchies
- premature UI work
- hidden allocations
- global mutable state
- business/product polish before the first plugin works

## Git workflow

Make small commits.

Suggested early commits:

1. Initial JUCE/CMake skeleton.
2. Minimal VST3 builds.
3. Audio passes through.
4. Add input/output gain parameters.
5. Add drive waveshaper.
6. Install/load successfully in Ableton.
7. Add first DSP tests or pluginval notes.

## Default assistant behavior

When generating code:

- Prefer complete files or clear patches.
- Keep the build working after every change.
- Mention commands to build and test.
- Point out real-time audio risks.
- Do not add dependencies unless needed.
- Do not silently change architecture.
- Ask before introducing neural modelling, installers, custom UI frameworks, licensing code, telemetry, or analytics.

When unsure, choose the simplest working solution that is safe for real-time audio.