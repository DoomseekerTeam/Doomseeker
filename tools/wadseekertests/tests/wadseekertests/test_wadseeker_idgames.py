import pytest

from wadseekertests.config import E2EConfig
from wadseekertests.wadseekerapp import WadseekerApp

class IdgamesArchive:
    # This should be a relatively small file as to not
    # torment the archive with large downloads.
    EXISTING_FILE = "bahdko1.wad"


@pytest.mark.idgames
def test_idgames_existing_file(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    # Given
    file = IdgamesArchive.EXISTING_FILE
    # When
    exitcode, _ = wadseeker.run_with_idgames(file)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / file).exists()


@pytest.mark.idgames
def test_idgames_missing_file(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    # Given
    file = (
        "dearhost_thisfiledoesnotexist_butistillneedto_"
        "searchitsometimes_totest_wadseeker.wad"
    )
    # When
    exitcode, _ = wadseeker.run_with_idgames(file)
    # Then
    assert exitcode == wadseeker.EC.FAIL.value
    assert not [f for f in e2econfig.workpath.iterdir()]  # empty dir
