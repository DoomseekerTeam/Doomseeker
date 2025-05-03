# Wadseeker End-to-End Tests

## Requirements

1. Astral `uv` Python manager installed.
2. A precompiled `wadseeker-app` from this repository.
3. A base `http-url` to a HTTP service with predefined endpoints.

## Running

Run in the directory with the `pyproject.toml` file:

```
uv run pytest --wadseeker-app=<wadseekerapp> --http-url=<url>
```

The standard pytest options (`-k`, `-s`) can also be used, but their
documentation is out of scope for this doc.

## Serving files

At the moment you may navigate to the `./data/http` directory and
run:

```
python3 -m http.server -b 127.0.0.1
```

Then adapt the `--http-url` argument to:

```
uv run pytest [...] --http-url=http://127.0.0.1:8000/
```

## How does it work?

Each test launches wadseekerapp set to bounce off a specific endpoint
of the HTTP server where expected resources must be provided by this
server. It then checks if the app behaved as expected at this endpoint.


## Copyright

Copyright (C) 2025 The Doomseeker Team

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without any warranty.
