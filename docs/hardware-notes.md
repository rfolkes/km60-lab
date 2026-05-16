# Hardware Notes — Boss KM-60

## Specifications (from service manual, Sep 1978)

- 6-channel compact line mixer
- Channel input: −50/−35/−20 dB switchable attenuator
- Input impedance: Lo 3.9 kΩ, Hi 33 kΩ
- Effects send/return: 0 dB / 300 Ω send, 0 dB / 10 kΩ return
- Output: +4 dBm (0 VU) into 300 Ω
- S/N ratio: >64 dB
- Power supply: ±14V rails (internal)

---

## Signal chain (per channel, AP-90 board)

```
Input jack
  → 3-position attenuator (−50/−35/−20 dB)
  → Input coupling cap (33 µF — DC block only, no audible HPF)
  → IC1 HA-1457 (preamp / input gain stage)
  → Treble EQ (Baxandall shelf, IC1 feedback network)
  → Bass EQ (Baxandall shelf)
  → Channel Volume fader (VR: 20 KB)
  → Pan pot (VR: 10 KB)
  → IC2 HA-1457 (effects send driver / bus feed)
  → Summing bus
```

Master bus (AP-91 board):
```
Summing bus → IC14–IC16 µPC4558C → Left/Right output (+4 dBm, 300 Ω)
```

---

## ICs

### HA-1457 (Hitachi) — channel strips, IC1–IC13

High-voltage, low-noise single op-amp designed for RIAA preamp use.

**Key specs (from Hitachi datasheet, at ±22.5V, 25°C):**
- Open-loop gain: 82 dB typical at 1 kHz (99 dB at DC)
- THD at 1 kHz, Vout=5V: **0.005% typical** (0.02% max)
- Output at THD=0.1%: **14.8V typical**, 13V min
- Output noise: 53 µV (Rg = 3.3 kΩ, 20 Hz–20 kHz)
- Supply voltage: ±25V max; used at ±14V in the KM-60

**In the KM-60 (±14V supply):**
Scaling from ±22.5V: clipping at ~9.2V peak = 6.5 Vrms.
Nominal output (+4 dBm = 1.74V peak) sits ~14 dB below clipping.

**THD vs. level shape (from datasheet graph):**
Essentially flat at ~0.005% from 0.2V to ~5V output, rising very gradually.
Only reaches 0.1% at ~13–15V (±22.5V supply) / ~9V (±14V in KM-60).
The onset is much more gradual than a tanh approximation — the chip is nearly
linear across most of its operating range.

**THD vs. frequency (from datasheet graph):**
At Vout=10V: THD rises steeply above ~5 kHz (slew-rate limiting).
At Vout=1V: flat and essentially unmeasurable to 50 kHz.
→ Heavy overdrive produces more harmonic distortion at high frequencies.

**Harmonic character:**
Push-pull class AB output stage → mix of H2 (asymmetric, from output stage imbalance)
and H3 (symmetric, from the nonlinearity). At normal levels, 82 dB of open-loop gain
and negative feedback suppress these to <0.005% — inaudible.
Near clipping: H2 dominant, followed by H3.

**DSP modelling implication:**
The HA-1457's character at normal mixer levels is almost entirely from the EQ curve,
not from saturation. The "drive" parameter in the plugin is a creative overload tool.
A circuit-accurate saturation model would need:
  - A soft-knee clipper, linear to ~70% of threshold, gentle slope above
  - A small H2 asymmetric component (from the push-pull stage)
  - HF roll-off under heavy drive (slew limiting above ~5 kHz)
These require measurement data to fit accurately.

### µPC4558C (NEC) — master bus, IC14–IC16

NEC dual op-amp, equivalent to the RC4558 / MC1458 family.
Slightly warmer/softer character than modern op-amps.
Used in the stereo summing bus and output stage.
Slightly different saturation character from the HA-1457 — to be confirmed by measurement.

---

## EQ — Baxandall tone control (from schematic)

Both channels use a Baxandall topology. Component values from the AP-90 schematic:

**Treble:**
- Pot: 50 kΩ log taper (VR11)
- Series resistors: R112 = R113 = 8.2 kΩ
- Caps: C101 = C108 = 0.012 µF (12 nF)
- Shelf turnover: f = 1/(2π × 8200 × 12e-9) = **1617 Hz** (implemented as 1600 Hz)
- Boost/cut range: ~±15 dB (estimated from pot/series resistor ratio); implemented as ±12 dB pending measurement

**Bass:**
- Pot: 50 kΩ log taper (VR12)
- Caps: C109 = C110 ≈ 0.0047 µF (possibly 0.047 µF — hard to read in scan)
- Shelf turnover: estimated **~350 Hz** (to be confirmed; see below)
- If caps are 0.047 µF with ~47 kΩ resistors: f ≈ 72 Hz (quite low)
- If caps are 0.0047 µF with ~47 kΩ resistors: f ≈ 720 Hz (quite high for bass)
- 350 Hz is the midpoint estimate; **measurement priority #1**

---

## Measurements to prioritise (when unit is available)

1. **Bass EQ turnover frequency** — the cap value ambiguity (0.0047 vs 0.047 µF) means
   the bass shelf could be anywhere from 70–700 Hz. One slow sweep with the bass at max
   will resolve this immediately.

2. **EQ boost/cut range** — confirm the ±12 dB range or adjust.

3. **THD vs. level at nominal** — confirm the ~14 dB headroom and
   the actual H2/H3 ratio for the waveshaper model.

4. **Noise floor character** — referenced against the datasheet's 53 µV noise figure.
