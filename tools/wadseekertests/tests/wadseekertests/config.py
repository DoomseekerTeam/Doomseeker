import sys
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

    def jobinspect(self) -> None:
        """Freeze the execution of code, await user input."""
        print("Job stopped; press ENTER to continue.\nDir:",
              self.workdir.name, file=sys.stderr)
        input()
