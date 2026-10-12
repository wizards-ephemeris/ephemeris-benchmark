# Contributing

Copyright (c) 2026 Magowie Sp. z o.o.

Send a patch that does one thing. Say which library you added or which comparison you fixed. Run `empheresis_tests` and, if you touched timing, the two benchmarks.

A new astrology library is one `Adapter` subclass registered in `src/Catalog.cpp`. The existing tests loop `subjects()` and compare each of them to the reference, so a registered library is measured by the suite you already have. Do not fork the assertions per vendor.

Do not add Swiss Ephemeris sources to this tree. The build fetches them. Do not copy their headers into a commit.

Identity essays are not part of a contribution. See `CODE_OF_CONDUCT.md`.
