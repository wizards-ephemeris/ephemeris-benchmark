#!/usr/bin/env python3
"""Draw the comparator charts from bench_position.json and bench_houses.json."""

import json
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = Path(__file__).resolve().parents[1]
CHARTS = ROOT / "charts"


def grouped(path, key, ylabel, title, out, skip_zero_error=False):
  data = json.loads(Path(path).read_text())
  labels = data["bodies"] if "bodies" in data else data["systems"]
  series = []
  for lib in data["libraries"]:
    values = lib[key]
    if skip_zero_error and max(values) == 0.0:
      continue
    series.append((lib["name"], values))
  if not series:
    raise SystemExit(f"no series in {path} for {key}")
  fig, ax = plt.subplots(figsize=(12, 5))
  width = 0.8 / len(series)
  xs = list(range(len(labels)))
  for i, (name, values) in enumerate(series):
    offset = (i - (len(series) - 1) / 2) * width
    ax.bar([x + offset for x in xs], values, width=width, label=name)
  ax.set_xticks(xs)
  ax.set_xticklabels(labels, rotation=45, ha="right")
  ax.set_ylabel(ylabel)
  ax.set_title(title)
  ax.legend()
  fig.tight_layout()
  CHARTS.mkdir(exist_ok=True)
  fig.savefig(CHARTS / out, dpi=140)
  plt.close(fig)
  print(CHARTS / out)


def main():
  pos = sys.argv[1] if len(sys.argv) > 1 else "bench_position.json"
  houses = sys.argv[2] if len(sys.argv) > 2 else "bench_houses.json"
  grouped(pos, "worstArcsec", "worst longitude error (arcsec)",
          "Longitude error versus Swiss Ephemeris, 1900-2100",
          "accuracy.png", skip_zero_error=True)
  grouped(pos, "nsPerCall", "nanoseconds per position",
          "Position cost, 1000 dates x body",
          "performance_position.png")
  grouped(houses, "nsPerCall", "nanoseconds per house call",
          "House cost, date x latitude x longitude",
          "performance_houses.png")


if __name__ == "__main__":
  main()
