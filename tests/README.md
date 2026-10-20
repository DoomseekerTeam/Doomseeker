# Unit tests

Doomseeker uses Qt Test for unit tests and CTest to run the test executables.

## Running tests

For build instructions, see [COMPILE.txt](../COMPILE.txt).

From your build directory, run all unit tests with:

```sh
ctest --output-on-failure -L unit
```

To run a specific suite, select it by name:

```sh
ctest --output-on-failure -R '^unit\.core\.strings$'
```

For multi-configuration builds, such as Visual Studio, add `-C Debug` (or the
configuration you built).

## Structure and conventions

- Mirror `src/` below `tests/unit/`: core tests belong in `unit/core/`, plugin
  tests in `unit/plugins/<plugin>/`, and Wadseeker unit tests in
  `unit/wadseeker/`.
- Name sources `tst_<subject>.cpp` and create one executable per subject.
- Follow the directory hierarchy with CMake files. Parent files add child
  directories; leaf files register tests with
  `add_doomseeker_unit_test(<subject> LIBRARIES <targets...>)`, for example
  `add_doomseeker_unit_test(strings LIBRARIES doomseeker_core)`.
- The function derives target names `doomseeker_test_<component>_<subject>`
  and CTest entries `unit.<component>.<subject>` from the directory hierarchy.
  For example, the strings suite is `doomseeker_test_core_strings` and
  `unit.core.strings`.
- CTest entries receive the `unit` and top-level component labels and a
  30-second timeout.
- Use Gherkin-style `Given`, `When`, and `Then` comment sections in all
  non-trivial tests to separate setup, execution, and assertions.
