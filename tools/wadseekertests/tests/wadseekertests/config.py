import tempfile
from dataclasses import dataclass
from pathlib import Path


@dataclass
class E2EConfig:
    wadseeker_app: Path
    http_url: str
    workdir: tempfile.TemporaryDirectory

    @property
    def workpath(self) -> Path:
        return Path(self.workdir.name)
