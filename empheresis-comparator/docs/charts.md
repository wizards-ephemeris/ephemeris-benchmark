# Reading the charts

`tools/make_charts.py` reads the JSON written by the two benchmarks and writes three PNGs into `charts/`.

| File | What it shows |
|------|----------------|
| `charts/accuracy.png` | Worst longitude error, in arcseconds, of each subject versus Swiss Ephemeris. The reference is omitted because its error against itself is zero. The sample is 1000 dates from 1900 to 2100. |
| `charts/performance_position.png` | Nanoseconds per `position` call, same date grid, both libraries. |
| `charts/performance_houses.png` | Nanoseconds per house call. Dates stop at 2050, matching the house comparison window. Latitudes include the high-latitude cases but not the pole, where some systems fall back. |

Swiss Ephemeris interpolates packed ephemeris files. Wizards Ephemeris evaluates series. The performance chart is expected to show Swiss ahead. That is a property of the method, not a bug in the timer.

Regenerate:

```
./bench_position bench_position.json
./bench_houses bench_houses.json
python3 tools/make_charts.py bench_position.json bench_houses.json
```
