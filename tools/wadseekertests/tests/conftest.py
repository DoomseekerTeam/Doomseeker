import os
import shutil
import sys
import tempfile
from pathlib import Path

import pytest

from wadseekertests.config import E2EConfig
from wadseekertests.datapath import DataPath, datapath
from wadseekertests.file import File
from wadseekertests.httpserver import httpserver
from wadseekertests.wadseekerapp import WadseekerApp


def pytest_addoption(parser):
    parser.addoption("--wadseeker-app", default="wadseeker", action="store",
                     help="Path to the Wadseeker app to test")


def pytest_make_parametrize_id(config, val, argname):
    if isinstance(val, File):
        return val.name
    return None


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
def other_fs_path(e2econfig: E2EConfig):
    """Yield an isolated directory on another filesystem and clean it on teardown."""
    if sys.platform != "linux":
        pytest.skip("Requires Linux /dev/shm")
    other_fs = Path("/dev/shm")
    if not other_fs.is_dir():
        pytest.skip("No /dev/shm filesystem available")

    try:
        target = tempfile.TemporaryDirectory(prefix="wadseeker-test-", dir=other_fs)
    except PermissionError:
        pytest.skip("Cannot create a test directory in /dev/shm")

    with target as directory:
        path = Path(directory)
        if path.stat().st_dev == e2econfig.workpath.stat().st_dev:
            pytest.skip("Requires two different filesystems")
        yield path


@pytest.fixture
def wadseeker(e2econfig: E2EConfig) -> WadseekerApp:
    yield WadseekerApp(
        exe=e2econfig.wadseeker_app,
        outputdir=e2econfig.workdir.name,
    )
