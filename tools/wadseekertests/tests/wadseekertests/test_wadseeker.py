from dataclasses import dataclass
from pathlib import Path
from urllib.parse import urlencode, urljoin

from wadseekertests.config import E2EConfig
from wadseekertests.wadseekerapp import WadseekerApp


@dataclass
class Endpoint:
    path: str

    def resolve_url(self, e2econfig: E2EConfig, query: dict[str, str] = None) -> str:
        url = urljoin(e2econfig.http_url, self.path)
        if query:
            url += "?" + urlencode(query)
        return url


class Endpoints:
    ATTACHMENT = Endpoint("attachment")
    ATTACHMENT_LINK_WAD = Endpoint("attachment_link_wad.html")
    ATTACHMENT_LINK_ZIP = Endpoint("attachment_link_zip.html")
    DIRECT_LINK_WAD = Endpoint("direct_link_wad.html")
    DIRECT_LINK_ZIP = Endpoint("direct_link_zip.html")
    LOOP_A = Endpoint("loop_a.html")
    LOOP_B = Endpoint("loop_b.html")
    MODBASE = Endpoint("modbase.html")
    MODBASE_AND_EXTRA = Endpoint("modbase_and_extra.html")
    MULTIFILE_MOD = Endpoint("multifile_mod.html")
    WAD_IS_ELSEWHERE = Endpoint("wad_is_elsewhere.html")
    ZEROLEN_ZIP_HEADER = Endpoint("0len_zip_header.html")
    ZIP_IS_ELSEWHERE = Endpoint("zip_is_elsewhere.html")


@dataclass
class File:
    strpath: str

    @property
    def name(self) -> str:
        return self.path.name

    @property
    def parent(self) -> Path:
        return self.path.parent

    @property
    def path(self) -> Path:
        return Path(self.strpath)

    def with_uppername(self) -> "File":
        return File(self.parent / self.name.upper())


class Files:
    CDUP_WAD = File("cdup.wad")
    CDUP_ZIP = File("cdup.zip")
    PLAIN_WAD = File("wadseeker_plain.wad")
    PLAIN_ZIP = File("wadseeker_plain.zip")
    MODBASE_WAD = File("modbase.wad")
    MODBASE_ZIP = File("modbase.zip")
    MODEXTRA_WAD = File("modextra.wad")
    MULTIFILE_MOD_WAD = File("multifile_mod.wad")
    MULTIFILE_MOD_ZIP = File("multifile_mod.zip")
    SUBDIR_WAD = File("subdir.wad")
    SUBDIR_ZIP = File("subdir.zip")
    SUBDIR_NONASCII_WAD = File("subdir_nonascii.wad")
    SUBDIR_NONASCII_ZIP = File("subdir_nonascii.zip")
    ZEROLEN_ZIP_HEADER_WAD = File("0len_zip_header.wad")
    ZEROLEN_ZIP_HEADER_ZIP = File("0len_zip_header.zip")


def test_e2econfig_sanity(e2econfig: E2EConfig):
    """If this test fails, something is wrong with the testing setup."""
    assert True


def test_wadseeker_sanity(wadseeker: WadseekerApp):
    """If this test fails, something is wrong with the testing setup."""
    assert True


def test_get_wad_from_html_direct_link(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    """Should find the file on a HTML site when a direct link to a WAD is present."""
    # Given
    site = Endpoints.DIRECT_LINK_WAD.resolve_url(e2econfig)
    assert not (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.PLAIN_WAD.name).exists()


def test_get_wad_from_html_direct_link_different_case(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    """Should find the file on a HTML site when a direct link
    to a WAD is present.

    The catch: we want "FILE.WAD", but site has "file.wad".
    """
    # Given
    site = Endpoints.DIRECT_LINK_WAD.resolve_url(e2econfig)
    assert not (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    assert Files.PLAIN_WAD.name.upper() != Files.PLAIN_WAD.name
    # When -- filename is upper-cased
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name.upper())
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.PLAIN_WAD.name.upper()).exists()


def test_get_zip_from_html_direct_link(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    """Should find the file on a HTML site when a direct link to a ZIP is present.

    This test expects a zip file to be downloaded and not extracted.
    """
    # Given
    site = Endpoints.DIRECT_LINK_ZIP.resolve_url(e2econfig)
    assert not (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    assert not (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_ZIP.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert not (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    assert (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()


def test_get_wad_directly(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    # Given
    site = Endpoint(Files.PLAIN_WAD.name).resolve_url(e2econfig)
    assert not (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.PLAIN_WAD.name).exists()


def test_get_wad_directly_different_case(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    # Given
    site = Endpoint(Files.PLAIN_WAD.name).resolve_url(e2econfig)
    assert not (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    assert Files.PLAIN_WAD.name.upper() != Files.PLAIN_WAD.name
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name.upper())
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.PLAIN_WAD.name.upper()).exists()


def test_get_zip_directly(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    # Given
    site = Endpoint(Files.PLAIN_ZIP.name).resolve_url(e2econfig)
    assert not (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_ZIP.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()


def test_get_zip_directly_different_case(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    # Given
    site = Endpoint(Files.PLAIN_ZIP.name).resolve_url(e2econfig)
    assert not (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()
    assert Files.PLAIN_ZIP.name.upper() != Files.PLAIN_ZIP.name
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_ZIP.name.upper())
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.PLAIN_ZIP.name.upper()).exists()


def test_extract_wad_from_zip_got_directly(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    # Given
    site = Endpoint(Files.PLAIN_ZIP.name).resolve_url(e2econfig)
    assert not (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert not (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()
    assert (e2econfig.workpath / Files.PLAIN_WAD.name).exists()


def test_extract_wad_from_zip_got_directly_different_case(
    e2econfig: E2EConfig,
    wadseeker: WadseekerApp,
):
    # Given
    site = Endpoint(Files.PLAIN_ZIP.name).resolve_url(e2econfig)
    assert not (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    assert Files.PLAIN_WAD.name.upper() != Files.PLAIN_WAD.name
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name.upper())
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert not (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()
    assert (e2econfig.workpath / Files.PLAIN_WAD.name.upper()).exists()


def test_extract_wad_from_zip_got_directly_but_zip_has_different_name(
    e2econfig: E2EConfig,
    wadseeker: WadseekerApp,
):
    """Look for "modextra.wad" in a directly linked "modbase.zip".

    There may be a case where the server provides a direct download link to an
    archive with name "mod.zip" and player already has "mod.wad", but the
    archive has multiple files. Inside is a file "mod-extra-v512.wad"
    that the player also needs, but won't find in a separate archive.

    Downloading "mod.zip" when we already have a "mod.wad" may be a goose chase,
    but there is no foresight in Wadseeker. If we need other files, the linked
    archive may or may not have them, but we should give the server a little
    benefit of the doubt and consider that the direct download link is put there
    for a reason.

    """
    # Given
    site = Endpoint(Files.MODBASE_ZIP.name).resolve_url(e2econfig)
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.MODEXTRA_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.MODEXTRA_WAD.name).exists()


def test_extract_wad_from_zip_of_same_name(
    e2econfig: E2EConfig,
    wadseeker: WadseekerApp,
):
    """When seeking a .wad, should download the .zip of the same
    name and extract it.

    The zip file should then be removed.
    """
    # Given
    site = Endpoints.DIRECT_LINK_ZIP.resolve_url(e2econfig)
    assert not (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    assert not (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    assert not (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()


def test_extract_wad_from_zip_of_same_name_but_different_case(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    """When seeking a .wad, should download the .zip of the same
    name and extract it.

    The zip file should then be removed.

    The catch: both the ZIP name and the WAD name are of different
    case than the found ones.
    """
    # Given
    site = Endpoints.DIRECT_LINK_ZIP.resolve_url(e2econfig)
    assert not (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    assert not (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()
    assert Files.PLAIN_WAD.name.upper() != Files.PLAIN_WAD.name
    assert Files.PLAIN_ZIP.name.upper() != Files.PLAIN_ZIP.name
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    assert not (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()
    assert not (e2econfig.workpath / Files.PLAIN_ZIP.name.upper()).exists()


def test_follow_a_wad_link_to_another_site(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    """This test encounters a link that leads to a different website that
    should serve the WAD.

    The link should be followed.
    """
    # Given
    site = Endpoints.WAD_IS_ELSEWHERE.resolve_url(e2econfig)
    assert not (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.PLAIN_WAD.name).exists()


def test_follow_a_zip_link_to_another_site_when_seeking_wad(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    """This test encounters a link that leads to a different website that
    should serve the ZIP file that should contain the seeked WAD.

    The link should be followed.
    """
    # Given
    site = Endpoints.ZIP_IS_ELSEWHERE.resolve_url(e2econfig)
    assert not (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    assert not (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    assert not (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()


def test_extract_zip_with_0len_file_entry(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    """Retrieve a zip file with no info on sizes and CRC
    in the file entry. This info is only in zip's central directory.

    A real zip file like this is here:
    https://www.doomworld.com/idgames/levels/doom/Ports/d-f/e1m8b
    """
    # Given
    site = Endpoints.ZEROLEN_ZIP_HEADER.resolve_url(e2econfig)
    assert not (e2econfig.workpath / Files.ZEROLEN_ZIP_HEADER_WAD.name).exists()
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.ZEROLEN_ZIP_HEADER_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.ZEROLEN_ZIP_HEADER_WAD.name).exists()


def test_extract_multifile_mod(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    # Given
    site = Endpoints.MULTIFILE_MOD.resolve_url(e2econfig)
    files = [
        Files.MULTIFILE_MOD_WAD,
        Files.MODEXTRA_WAD,
    ]
    # When
    exitcode, _ = wadseeker.run_with_site(site, [f.name for f in files])
    # Then
    assert exitcode == wadseeker.EC.OK.value
    for file in files:
        assert (e2econfig.workpath / file.name).exists()


def test_extract_multifile_mod_but_the_main_file_is_missing(
    e2econfig: E2EConfig,
    wadseeker: WadseekerApp,
):
    """Wadseeker should fail, but still install the files it can find."""
    # Given
    site = Endpoints.MODBASE.resolve_url(e2econfig)
    files = [
        Files.MODBASE_WAD,
        Files.MODEXTRA_WAD,
    ]
    # When
    exitcode, _ = wadseeker.run_with_site(site, [f.name for f in files])
    # Then
    assert exitcode == wadseeker.EC.FAIL.value
    assert not (e2econfig.workpath / Files.MODBASE_WAD.name).exists()
    assert (e2econfig.workpath / Files.MODEXTRA_WAD.name).exists()


def test_extract_multifile_mod_with_files_swapped(
    e2econfig: E2EConfig,
    wadseeker: WadseekerApp,
):
    """The individual archives have the files mixed up.

    The endpoint here provides two archives that *technically* contain all the
    files we wish to download, but archive "base" contains file "extra", and
    archive "extra" contains file "base". It's a mix-up. This is an odd case
    for Wadseeker, because Wadseeker avoids downloading files that it thinks
    will not produce good results. A potentially "good result" file may be
    discarded because its associated modfile was installed in some other manner
    already.

    So, with such a mix-up, when Wadseeker downloads one of the archives and
    finds the other file there, it installs it, and then it may no longer want
    to download the other archive, because it is associated with the file it has
    just installed. It doesn't have any foresight that the other archive will
    contain the previous file. Discarding such an archive is a valid result.

    However, because all the files are *technically* there, the final valid
    result may also be that all the files become installed by some chance.

    So, in this test there are 3 possible valid results:

    - (A) EC.OK; both files are installed.
    - (B) EC.FAIL; only "base" file is installed.
    - (C) EC.FAIL; only "extra" file is installed.

    """
    # Given
    site = Endpoints.MODBASE_AND_EXTRA.resolve_url(e2econfig)
    files = [
        Files.MODBASE_WAD,
        Files.MODEXTRA_WAD,
    ]
    # When
    exitcode, _ = wadseeker.run_with_site(site, [f.name for f in files])
    # Then
    if exitcode == wadseeker.EC.OK.value:
        # case (A)
        assert (e2econfig.workpath / Files.MODBASE_WAD.name).exists()
        assert (e2econfig.workpath / Files.MODEXTRA_WAD.name).exists()
    else:
        # case (B) or (C)
        assert exitcode == wadseeker.EC.FAIL.value
        if (e2econfig.workpath / Files.MODBASE_WAD.name).exists():
            # case (B)
            assert not (e2econfig.workpath / Files.MODEXTRA_WAD.name).exists()
        else:
            # case (C)
            assert (e2econfig.workpath / Files.MODEXTRA_WAD.name).exists()


def test_extract_from_zip_subdirectory(
    e2econfig: E2EConfig,
    wadseeker: WadseekerApp,
):
    """WAD inside the archive is nested in a subdirectory."""
    # Given
    site = Endpoint(Files.SUBDIR_ZIP.name).resolve_url(e2econfig)
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.SUBDIR_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.SUBDIR_WAD.name).exists()


def test_extract_from_zip_subdirectory_nonascii(
    e2econfig: E2EConfig,
    wadseeker: WadseekerApp,
):
    """WAD inside the archive is nested in a subdirectory.

    The catch: the subdirectory name uses non-ascii characters.
    """
    # Given
    site = Endpoint(Files.SUBDIR_NONASCII_ZIP.name).resolve_url(e2econfig)
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.SUBDIR_NONASCII_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.SUBDIR_NONASCII_WAD.name).exists()


def test_extract_from_zip_with_path_traversal(
    e2econfig: E2EConfig,
    wadseeker: WadseekerApp,
):
    """The zip file contains a nasty surprise where the WAD
    is placed in a directory named "..".

    Wadseeker should just extract the WAD file to the target directory
    regardless of the subdirectories in the archive, even if those
    subdirectories are malicious.
    """
    # Given: the target directory should be in a subdir of the workpath
    # because if things go wrong, the WAD will be extracted one directory
    # above the target one.
    subworkpath = e2econfig.workpath / "cdup_subdir"
    subworkpath.mkdir()
    site = Endpoint(Files.CDUP_ZIP.name).resolve_url(e2econfig)
    wadseeker.outputdir = str(subworkpath)
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.CDUP_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (subworkpath / Files.CDUP_WAD.name).exists()
    assert not (e2econfig.workpath / Files.CDUP_WAD.name).exists()


def test_attachment_direct_wad(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    # Given
    site = Endpoints.ATTACHMENT.resolve_url(e2econfig, query={
        "file": Files.PLAIN_WAD.name
    })
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.PLAIN_WAD.name).exists()


def test_attachment_extract_wad_from_direct_zip(
    e2econfig: E2EConfig,
    wadseeker: WadseekerApp,
):
    # Given
    site = Endpoints.ATTACHMENT.resolve_url(e2econfig, query={
        "file": Files.PLAIN_ZIP.name,
    })
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    assert not (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()


def test_attachment_wad_linked_from_html(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    # Given
    site = Endpoints.ATTACHMENT_LINK_WAD.resolve_url(e2econfig)
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.PLAIN_WAD.name).exists()


def test_attachment_zip_linked_from_html_and_extracted(
    e2econfig: E2EConfig,
    wadseeker: WadseekerApp,
):
    # Given
    site = Endpoints.ATTACHMENT_LINK_ZIP.resolve_url(e2econfig)
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name)
    # Then
    assert exitcode == wadseeker.EC.OK.value
    assert (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
    assert not (e2econfig.workpath / Files.PLAIN_ZIP.name).exists()


def test_should_exit_loops(e2econfig: E2EConfig, wadseeker: WadseekerApp):
    """A broken site sends Wadseeker into a site navigation loop."""
    # Given
    site = Endpoints.LOOP_A.resolve_url(e2econfig)
    # When
    exitcode, _ = wadseeker.run_with_site(site, Files.PLAIN_WAD.name, timeout = 10.0)
    # Then: failure is expected, but not a timeout
    assert exitcode == wadseeker.EC.FAIL.value
    assert not (e2econfig.workpath / Files.PLAIN_WAD.name).exists()
