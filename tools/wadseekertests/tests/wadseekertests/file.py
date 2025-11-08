from dataclasses import dataclass
from pathlib import Path


@dataclass
class FileChecksum:
    algorithm: str
    hexvalue: str


@dataclass
class File:
    strpath: str
    checksums: list[FileChecksum] = None

    @property
    def name(self) -> str:
        return self.path.name

    @property
    def namechecksum(self) -> str:
        checksum = self.checksums[0]
        return f"{self.name}:{checksum.algorithm}:{checksum.hexvalue}"

    @property
    def parent(self) -> Path:
        return self.path.parent

    @property
    def path(self) -> Path:
        return Path(self.strpath)

    def with_uppername(self) -> "File":
        return File(self.parent / self.name.upper())
