# Wadseeker unit tests

These tests run in both the full Doomseeker build and the standalone Wadseeker
source package. Enable `BUILD_TESTING` when configuring; Qt Test is required
only when testing is enabled.

From your build directory, run the Wadseeker unit tests with:

```sh
ctest --output-on-failure -L wadseeker
```

For multi-configuration builds, such as Visual Studio, add `-C Debug` (or the
configuration you built).
