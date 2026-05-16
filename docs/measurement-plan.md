# Measurement Plan

Planned captures once the Boss KM-60 unit is available.

## Tools

- REW (Room EQ Wizard) for frequency sweeps and THD
- Focusrite or similar interface for capture
- Python (scipy, numpy, matplotlib) for analysis

## Capture list

| # | Measurement         | Settings                         | Purpose                        |
|---|---------------------|----------------------------------|--------------------------------|
| 1 | Frequency response  | Gain at unity, low level         | Baseline EQ fingerprint        |
| 2 | Frequency response  | Gain at +10 dB                   | Saturation-induced colouring   |
| 3 | THD+N vs level      | Sweep input from −30 to 0 dBu   | Saturation onset point         |
| 4 | Harmonic profile    | Sine at −10 dBu, several freqs   | Even/odd harmonic character    |
| 5 | Noise floor         | No input, fader at unity         | Noise model baseline           |
| 6 | Crosstalk           | One channel active, measure rest | Channel isolation               |
| 7 | Clipping behaviour  | Input beyond rated max           | Hard vs soft clip shape        |
| 8 | Send/return path    | Looped back                      | Effects loop colouring          |

## Storage

Raw captures go in `measurements/captures/`.
Analysis scripts go in `measurements/scripts/`.
Processed results and plots go in `measurements/analysis/`.
