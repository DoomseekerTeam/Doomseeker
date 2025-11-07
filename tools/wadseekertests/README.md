# Wadseeker End-to-End Tests

## Requirements

1. Astral `uv` Python manager installed.
2. A precompiled `wadseeker-app` from this repository.

## Running

Run in the directory with the `pyproject.toml` file:

```
uv run pytest --wadseeker-app=<wadseekerapp>
```

Use pytest options `-v` and `-s` to get more logs and `-k <pattern>`
to limit the execution to the selected tests.

## How does it work?

Each test launches wadseekerapp set to bounce off a specific endpoint
of the HTTP server where expected resources must be provided by this
server. It then checks if the app behaved as expected at this endpoint.

The HTTP server is self-hosted by the test framework for the runtime
of the tests. The files are served from `data/http` directory. The
server is bound to `127.0.0.1` with an auto-selected port.

## Copyright

Copyright (C) 2025 The Doomseeker Team

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without any warranty.
