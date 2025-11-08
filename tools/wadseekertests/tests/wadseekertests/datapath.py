from dataclasses import dataclass
from pathlib import Path

import pytest

@dataclass
class DataPath:
    root: Path

    @property
    def http(self) -> Path:
        return self.root / "http"


@pytest.fixture(scope="session")
def datapath(request: pytest.FixtureRequest) -> DataPath:
    return DataPath(request.config.rootpath / "data")
