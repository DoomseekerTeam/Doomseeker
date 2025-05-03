import os
from dataclasses import dataclass
from pathlib import Path
from urllib.parse import urljoin

from wadseekertests.config import E2EConfig
from wadseekertests.wadseekerapp import WadseekerApp


@dataclass
class Endpoint:
    path: str

    def resolve_url(self, e2econfig: E2EConfig):
        return urljoin(e2econfig.http_url, self.path)


class Endpoints:
    DIRECT_LINK = Endpoint("direct_link.html")


@dataclass
class File:
    name: str

    @property
    def path(self) -> Path:
        return Path(self.name)


class Files:
    PLAIN_WAD = File("wadseeker_plain.wad")


def test_e2econfig_sanity(e2econfig: E2EConfig):
    """If this test fails, something is wrong with the testing setup."""
    assert True


def test_wadseeker_sanity(wadseeker: WadseekerApp):
    """If this test fails, something is wrong with the testing setup."""
    assert True


def test_direct_link(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    """Should find the file on a HTML site when a direct link is present."""
    site = Endpoints.DIRECT_LINK.resolve_url(e2econfig)
    assert not (e2econfig.workpath / Files.PLAIN_WAD.path).exists()
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name)
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.PLAIN_WAD.path).exists()
