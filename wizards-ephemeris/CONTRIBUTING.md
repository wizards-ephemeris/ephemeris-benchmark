# Contributing

Copyright (c) 2026 Magowie Sp. z o.o.

This repository is the library only. It does not fetch or link Swiss Ephemeris. Comparison tests live in Empheresis Comparator.

A change belongs here when it alters positions, frames, houses, or the public headers under `include/eph/`. Keep the C++20 surface. Do not add a dependency. Generated coefficients in `src/generated/` come from `tools/gen_series.py`; change the generator and regenerate rather than hand-editing a table.

State the accuracy consequence in `docs/accuracy.md` when a change moves a published bound.

Identity essays are not part of a contribution. See `CODE_OF_CONDUCT.md`.
