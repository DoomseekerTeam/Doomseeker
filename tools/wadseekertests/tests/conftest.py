import os
import shutil
import sys
import tempfile
from pathlib import Path

import pytest

from wadseekertests.config import E2EConfig
from wadseekertests.wadseekerapp import WadseekerApp


def pytest_addoption(parser):
    parser.addoption("--wadseeker-app", default="wadseeker", action="store",
                     help="Path to the Wadseeker app to test")
    parser.addoption("--http-url", default="http://127.0.0.1/",
                     help="Base URL to the HTTP server with test data")


@pytest.fixture
def e2econfig(pytestconfig) -> E2EConfig:
    workdir = tempfile.TemporaryDirectory(prefix="wadseeker-test")
    try:
        # Structurize options.
        config = E2EConfig(
            wadseeker_app=Path(pytestconfig.getoption("wadseeker_app")),
            http_url=pytestconfig.getoption("http_url"),
            workdir=workdir,
        )
        # Validate config.
        app_path = shutil.which(config.wadseeker_app)
        if app_path is None:
            app_path = config.wadseeker_app
        if app_path is None or not os.path.exists(app_path):
            pytest.fail(f"'{config.wadseeker_app}' executable not found")
        if not os.access(app_path, os.X_OK):
            pytest.fail(f"'{config.wadseeker_app}' cannot be run")

        print("Workdir: ", workdir.name, file=sys.stderr)
        yield config
    finally:
        workdir.cleanup()


@pytest.fixture
def wadseeker(e2econfig: E2EConfig) -> WadseekerApp:
    yield WadseekerApp(
        exe=e2econfig.wadseeker_app,
        outputdir=e2econfig.workdir.name,
    )
