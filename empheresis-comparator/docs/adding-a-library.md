# Adding a library

The comparator is a list of `Adapter` implementations. Swiss Ephemeris is the reference. Every other entry in `subjects()` is scored against it.

1. Subclass `emph::Adapter` (`include/Adapter.hpp`). Implement `name`, `available`, `position`, `equatorial`, `houses`, `ayanamsaDeg`, both `setSidereal` overloads, and `setLilithMode`.
2. Register the instance in `libraries()` and, unless it is the reference, in `subjects()` inside `src/Catalog.cpp`.
3. Rebuild `empheresis_tests`. The position, house, ayanamsa, and sidereal cases loop the subject list. A library that cannot meet a bound fails that case. Adjust a bound only when you can show the difference is the theory, and write the reason in the library's own accuracy note.
4. Rebuild `bench_position` and `bench_houses`, rerun them, and regenerate `charts/` with `tools/make_charts.py`.

`available()` may return false when a data file is missing. The suite still constructs the adapter; skip work inside the methods rather than throwing past the harness.

Do not special-case the new name inside each test. The point of the catalog is that the tests stay generic.
