"""
KM-60 EQ shelf analysis.

Takes REW frequency-response text exports and finds:
  - Shelf turnover frequency (the -3 dB point of the shelf relative to its maximum)
  - Maximum boost / cut range

Usage:
    python analyse_eq.py <flat.txt> <bass_boost.txt> <treble_boost.txt>

REW export: in REW, right-click a measurement → Export → Export as text.
"""

import sys
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

FMIN = 20.0
FMAX = 20000.0
PASSBAND_LO = 200.0
PASSBAND_HI = 2000.0


# ── REW file loader ───────────────────────────────────────────────────────────

def load_rew(path: str) -> tuple[np.ndarray, np.ndarray]:
    """
    Load a REW frequency-response text export.
    Returns (frequencies_hz, levels_db).
    REW puts comment lines starting with '*'; data is tab- or space-separated.
    """
    freqs, levels = [], []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("*"):
                continue
            parts = line.split()
            if len(parts) < 2:
                continue
            try:
                freqs.append(float(parts[0]))
                levels.append(float(parts[1]))
            except ValueError:
                continue
    return np.array(freqs), np.array(levels)


def band_limit(freqs: np.ndarray, levels: np.ndarray,
               fmin: float = FMIN, fmax: float = FMAX
               ) -> tuple[np.ndarray, np.ndarray]:
    """Restrict arrays to the audible band."""
    mask = (freqs >= fmin) & (freqs <= fmax)
    return freqs[mask], levels[mask]


def normalise_passband(freqs: np.ndarray, rel: np.ndarray) -> np.ndarray:
    """
    Shift the relative curve so that the passband (200 Hz–2 kHz) sits at 0 dB.
    This removes the absolute level difference between measurements taken at
    different REW output levels (e.g. flat at -12 dBFS vs. EQ at -24 dBFS).
    """
    mask = (freqs >= PASSBAND_LO) & (freqs <= PASSBAND_HI)
    if not mask.any():
        return rel
    return rel - np.median(rel[mask])


# ── Analysis ──────────────────────────────────────────────────────────────────

def find_shelf_turnover(freqs: np.ndarray, levels: np.ndarray,
                        shelf: str) -> tuple[float, float]:
    """
    Find the shelf turnover frequency and max boost/cut.

    shelf: 'low'  — bass shelf, max boost is at low frequencies
           'high' — treble shelf, max boost is at high frequencies

    The turnover is defined as the frequency where the boost/cut
    is 3 dB below its maximum, on the side that transitions to unity.

    Expects freqs/levels already restricted to the audible band and
    with the passband normalised to 0 dB.
    """
    max_boost = np.max(levels)
    target = max_boost - 3.0

    if shelf == "low":
        # Maximum is at low frequencies; scan from high → low for the 3 dB point.
        for i in range(len(freqs) - 1, -1, -1):
            if levels[i] >= target:
                if i + 1 < len(freqs):
                    f0, f1 = freqs[i], freqs[i + 1]
                    l0, l1 = levels[i], levels[i + 1]
                    t = (target - l0) / (l1 - l0) if l1 != l0 else 0.0
                    return float(f0 + t * (f1 - f0)), float(max_boost)
                return float(freqs[i]), float(max_boost)

    elif shelf == "high":
        # Maximum is at high frequencies; scan from low → high for the 3 dB point.
        for i in range(1, len(freqs)):
            if levels[i] >= target:
                f0, f1 = freqs[i - 1], freqs[i]
                l0, l1 = levels[i - 1], levels[i]
                t = (target - l0) / (l1 - l0) if l1 != l0 else 0.0
                return float(f0 + t * (f1 - f0)), float(max_boost)

    raise ValueError(f"Could not find -3 dB turnover point for shelf='{shelf}'")


# ── Plotting ──────────────────────────────────────────────────────────────────

def plot_sweeps(captures: list, rel_captures: list = None) -> None:
    rows = 2 if rel_captures else 1
    fig, axes = plt.subplots(rows, 1, figsize=(10, 5 * rows))
    if rows == 1:
        axes = [axes]

    ax = axes[0]
    for label, freqs, levels in captures:
        ax.semilogx(freqs, levels, label=label)
    ax.set_xlabel("Frequency (Hz)")
    ax.set_ylabel("Level (dB)")
    ax.set_title("KM-60 EQ sweeps (raw)")
    _format_ax(ax)

    if rel_captures:
        ax2 = axes[1]
        for label, freqs, levels in rel_captures:
            ax2.semilogx(freqs, levels, label=label)
        ax2.set_xlabel("Frequency (Hz)")
        ax2.set_ylabel("Relative level (dB)")
        ax2.set_title("KM-60 EQ sweeps (relative to flat, passband-normalised)")
        _format_ax(ax2)

    plt.tight_layout()
    out = Path("measurements/analysis/eq_sweeps.png")
    out.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(out, dpi=150)
    print(f"\nPlot saved → {out}")


def _format_ax(ax) -> None:
    ax.set_xlim(FMIN, FMAX)
    ax.set_xticks([20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000])
    ax.set_xticklabels(["20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k"])
    ax.grid(True, which="both", alpha=0.3)
    ax.axhline(0, color="black", linewidth=0.8)
    ax.legend()


# ── Main ──────────────────────────────────────────────────────────────────────

def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    paths = sys.argv[1:]
    captures = []
    rel_captures = []

    flat_freqs = None
    flat_levels = None

    for path in paths:
        name = Path(path).stem
        freqs, levels = load_rew(path)
        captures.append((name, freqs, levels))

        if "flat" in name.lower():
            flat_freqs = freqs
            flat_levels = levels
            bf, bl = band_limit(freqs, levels)
            print(f"\n── {name} (baseline) ──")
            print(f"   Level range: {bl.min():.1f} to {bl.max():.1f} dB")
            pb_mask = (bf >= PASSBAND_LO) & (bf <= PASSBAND_HI)
            print(f"   Mean 200 Hz–2 kHz: {np.mean(bl[pb_mask]):.2f} dB")

        elif "bass" in name.lower() and "boost" in name.lower():
            print(f"\n── {name} ──")
            if flat_levels is not None and len(flat_levels) == len(levels):
                rel = levels - flat_levels
            else:
                rel = levels
            bf, br = band_limit(freqs, rel)
            br_norm = normalise_passband(bf, br)
            rel_captures.append((name + " (rel)", bf, br_norm))
            try:
                fc, max_b = find_shelf_turnover(bf, br_norm, shelf="low")
                print(f"   Bass shelf turnover: {fc:.0f} Hz")
                print(f"   Max boost: {max_b:.1f} dB")
                print(f"\n   → Update in Km60Processor.cpp:")
                print(f"     static constexpr double kBassHz = {fc:.0f}.0;")
            except ValueError as e:
                print(f"   Could not determine turnover: {e}")

        elif "treble" in name.lower() and "boost" in name.lower():
            print(f"\n── {name} ──")
            if flat_levels is not None and len(flat_levels) == len(levels):
                rel = levels - flat_levels
            else:
                rel = levels
            bf, br = band_limit(freqs, rel)
            br_norm = normalise_passband(bf, br)
            rel_captures.append((name + " (rel)", bf, br_norm))
            try:
                fc, max_b = find_shelf_turnover(bf, br_norm, shelf="high")
                print(f"   Treble shelf turnover: {fc:.0f} Hz")
                print(f"   Max boost: {max_b:.1f} dB")
                print(f"\n   → Update in Km60Processor.cpp:")
                print(f"     static constexpr double kTrebleHz = {fc:.0f}.0;")
            except ValueError as e:
                print(f"   Could not determine turnover: {e}")

        elif "bass" in name.lower() and "cut" in name.lower():
            print(f"\n── {name} ──")
            if flat_levels is not None and len(flat_levels) == len(levels):
                rel = levels - flat_levels
                bf, br = band_limit(freqs, rel)
                br_norm = normalise_passband(bf, br)
                rel_captures.append((name + " (rel)", bf, br_norm))
                print(f"   Max cut: {br_norm.min():.1f} dB")
            else:
                bf, bl = band_limit(freqs, levels)
                print(f"   Max cut (raw): {bl.min():.1f} dB")

        elif "treble" in name.lower() and "cut" in name.lower():
            print(f"\n── {name} ──")
            if flat_levels is not None and len(flat_levels) == len(levels):
                rel = levels - flat_levels
                bf, br = band_limit(freqs, rel)
                br_norm = normalise_passband(bf, br)
                rel_captures.append((name + " (rel)", bf, br_norm))
                print(f"   Max cut: {br_norm.min():.1f} dB")
            else:
                bf, bl = band_limit(freqs, levels)
                print(f"   Max cut (raw): {bl.min():.1f} dB")

    plot_sweeps(captures, rel_captures if rel_captures else None)


if __name__ == "__main__":
    main()
