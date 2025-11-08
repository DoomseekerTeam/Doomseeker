import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path

from wadseekertests.datapath import DataPath


@dataclass
class E2EConfig:
    """Aggregator of state and parameters useful in all E2E tests.

    It binds the more specific fixtures together and makes them available in one
    place. E2EConfig should be created by a fixture that receives all the basic
    fixtures and parameters and puts them together into an object of this class.

    """
    wadseeker_app: Path
    http_url: str
    workdir: tempfile.TemporaryDirectory
    datapath: DataPath

    @property
    def workpath(self) -> Path:
        return Path(self.workdir.name)

    def jobinspect(self) -> None:
        """Freeze the execution of code, await user input."""
        print("Job stopped; press ENTER to continue.\nDir:",
              self.workdir.name, file=sys.stderr)
        input()
