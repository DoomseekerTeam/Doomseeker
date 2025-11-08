import os
import shutil
import sys
import tempfile
from pathlib import Path

import pytest

from wadseekertests.config import E2EConfig
from wadseekertests.datapath import DataPath, datapath
from wadseekertests.httpserver import httpserver
from wadseekertests.wadseekerapp import WadseekerApp


def pytest_addoption(parser):
    parser.addoption("--wadseeker-app", default="wadseeker", action="store",
                     help="Path to the Wadseeker app to test")


def pytest_collection_modifyitems(config, items):
    # Tests that require the /idgames archive are disabled by default.
    _set_marker_as_explicit(config, items, "idgames")


def _set_marker_as_explicit(config, items, marker: str) -> None:
    if config.getoption("-m") and marker in config.getoption("-m"):
        return
    skip = pytest.mark.skip(reason=f"test must be enabled explicitly with '-m {marker}'")
    for item in items:
        if marker in item.keywords:
            item.add_marker(skip)


@pytest.fixture
def e2econfig(pytestconfig, datapath: DataPath, httpserver: str) -> E2EConfig:
    workdir = tempfile.TemporaryDirectory(prefix="wadseeker-test")
    try:
        # Structurize options.
        config = E2EConfig(
            wadseeker_app=Path(pytestconfig.getoption("wadseeker_app")),
            http_url=httpserver,
            workdir=workdir,
            datapath=datapath,
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
