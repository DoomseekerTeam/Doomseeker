#!/usr/bin/python3
#
# ------------------------------------------------------------------------------
# build-update-kit.py
# ------------------------------------------------------------------------------
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301  USA
#
# ------------------------------------------------------------------------------
# Copyright (C) 2024 "Zalewa" <zalewapl@gmail.com>
# ------------------------------------------------------------------------------
"""Split Doomseeker installation into the individual update packages.

This script is a part of Doomseeker's update packages auto-build system.

What this does:

1. Runs Doomseeker located in BINARY_DIR with `--version-json` arg and collects
   the output with packages versions.

2. Creates an output directory for the packaged packages in the current working
   directory. This directory is named after the update channel and the current
   timestamp, but is also slightly randomized to ensure that we're starting
   with an empty output dir.

3. Runs "create-package.py" script for every package. All information the
   script needs is passed in via command line arguments. All heavy lifting
   occurs in this script. It collects the files belonging to the package and
   creates two files in the output: the archive with the package's files and
   the metadata file describing the package. The output files are named after
   the internal package name, the human-readable version, revision, channel and
   the platform.

4. If at least one package was created successfully, this script creates an
   "update-info_<platform>_<channel>.js" file in the output dir. This file
   contains information on all the packaged packages. Doomseeker downloads this
   file each time it wants to check if there are any new updates. OVERWRITE
   THIS FILE ON THE SERVER WITH ABSOLUTE CARE!!! The file is formatted in a
   human-readable fashion and is suitable for diffing. Make sure not to upload
   a bogus file, otherwise you may screw up people's setups.

Requirements:

- Python 3.8+,

- this script calls "create-package.py" located in its own directory, so its
  requirements must be met,

- config dir for the current platform must exist and contain JSON packaging
  configs. One config per package. An example structure of this JSON can be
  found in config-template.js. The config files must be named after internal
  package names (doomseeker-core.js, p-chocolatedoom.js, etc.). The current
  platform is determined at runtime;

- Doomseeker binary packages need to be compiled beforehand and placed in
  a directory in a structure that is the same as after deployment on end-user
  system. All Doomseeker requirements and peripheral files must also be
  located in this directory (translations, plugins, runtime DLLs, etc.).
  `make install/strip`, or INSTALL target in Visual Studio, should take
  care of creating appropriate structure.

"""
import argparse
import json
import logging
import os
import platform
import random
import shlex
import sys
import subprocess
import tempfile
from dataclasses import dataclass
from datetime import datetime
from enum import Enum
from pathlib import Path
from typing import Dict, List, Optional, TextIO, Tuple


logger = logging.getLogger(__name__)

EC_OK = 0
EC_ERROR = 1
EC_FAIL = 3
EC_BADUSE = 4

URL_BASE = "https://doomseeker.drdteam.org/updates"
PKG_BLACKLIST = ["doomseeker"]


@dataclass
class Platform:
    name: str
    doomseeker_exe: str

    @property
    def package_configs_dir(self) -> str:
        return f"{self.name}-configs"


class Platforms(Enum):
    WIN32 = Platform("win32", "doomseeker.exe")
    MACOSX = Platform("macosx", "Contents/MacOS/doomseeker")

    @classmethod
    def current(cls) -> Platform:
        d = {
            "Windows": cls.WIN32,
            "Darwin": cls.MACOSX
        }
        return d.get(platform.system()).value


@dataclass
class Package:
    name: str
    display_name: str
    display_version: Optional[str]
    revision: str

    @property
    def shown_version(self) -> str:
        return self.display_version or self.revision

    @classmethod
    def from_doomseeker(cls, name, data: Dict[str, str]):
        return cls(
            name=name,
            display_name=data.get("display-name", ""),
            display_version=data.get("display-version") or None,
            revision=data.get("revision", "")
        )


VersionInfo = List[Package]


def package_suffix(pkg: Package, platform: Platform, channel: str) -> str:
    suffix = f"{pkg.revision}-{channel}_{platform.name}"
    if pkg.display_version and pkg.display_version != pkg.revision:
        suffix = f"{pkg.display_version}_{suffix}"
    return f"_{suffix}"


def package_archive_name(pkg: Package, platform: Platform,
                         channel: str) -> str:
    return f"{pkg.name}{package_suffix(pkg, platform, channel)}.zip"


def process_package(pkg: Package, platform: Platform, channel: str,
                    binary_dir: Path, output_dir: Path):
    logger.info("Processing package: %s", pkg.name)

    suffix = package_suffix(pkg, platform, channel)
    script_dir = Path(__file__).parent
    config_file = script_dir / platform.package_configs_dir / f"{pkg.name}.js"
    create_package_script = script_dir / "create-package.py"
    if not create_package_script.exists():
        raise FileNotFoundError(f"script '{create_package_script}' not found")

    subprocess.check_call([
        sys.executable,
        str(create_package_script),
        "-p", platform.name,
        "-V", pkg.shown_version,
        "--suffix", suffix,
        str(binary_dir),
        str(config_file),
        str(output_dir),
    ])


def dump_update_info(output: TextIO, platform: Platform, channel: str,
                     version_info: VersionInfo):
    """Write out the update-info metadata."""
    update_info = {}
    for pkg in version_info:
        archive_name = package_archive_name(pkg, platform, channel)
        url = f"{URL_BASE}/{platform.name}/{channel}/{archive_name}"
        update_info[pkg.name] = {
            "display-name": pkg.display_name,
            "display-version": pkg.shown_version,
            "revision": pkg.revision,
            "URL": url
        }
    json.dump(update_info, output, indent=2)
    output.write("\n")


def obtain_version_information(doomseeker: Path) -> VersionInfo:
    with tempfile.NamedTemporaryFile(mode="w", delete=False) as file:
        # On Windows, opening for writing a file that is already open in
        # another process will fail by default. So, let's just create an empty
        # file here and immediately close it. Then let Doomseeker write the
        # versions at its path. Afterwards we can reopen this path, collect
        # what Doomseeker has written, and then delete the file.
        pass
    try:
        versionfile = Path(file.name)
        cmd = [doomseeker, "--version-json", str(versionfile)]
        logger.debug("Running Doomseeker: %s",
                     ' '.join(shlex.quote(str(arg)) for arg in cmd))
        subprocess.check_call(cmd)
        with versionfile.open("r") as io:
            pkg_data = json.load(io)
    finally:
        versionfile.unlink()
    return [Package.from_doomseeker(name, data)
            for name, data in pkg_data.items()]


def spawn_unique_dir(prefix: str) -> Path:
    stamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    padding = "".join(random.choice("0123456789abcdef") for _ in range(8))
    name = f"{prefix}-{stamp}-{padding}"
    os.makedirs(name)
    return Path(name)


def get_system_platform() -> Platform:
    sysplatform = Platforms.current()
    if sysplatform is None:
        print("error: unsupported platform:", platform.system(),
              file=sys.stderr)
        sys.exit(1)
    return sysplatform


def configure_logging(level: int = logging.INFO):
    logger = logging.getLogger()
    formatter = logging.Formatter("%(levelname)-8s: %(message)s")
    handler = logging.StreamHandler()
    handler.setLevel(level)
    handler.setFormatter(formatter)
    logger.addHandler(handler)
    logger.setLevel(level)


def verbosity_to_loglevel(verbose: int, quiet: int) -> int:
    verbosity = verbose - quiet
    default_level = logging.INFO
    # In Python, lower levels are more verbose.
    level_delta = logging.INFO - logging.DEBUG
    return max(1, default_level - (verbosity * level_delta))


def parse_args():
    argp = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    argp.add_argument("-c", "--channel", dest="update_channel", required=True,
                      action="store", choices=['beta', 'stable', 'sojourner'],
                      help="update channel to create packages for")
    argp.add_argument("-i", "--input", dest="binary_dir", required=True,
                      type=Path,
                      help=("directory with the deployed Doomseeker from "
                            "which packages will be created"))
    argp.add_argument("-q", "--quiet", action="count", default=0)
    argp.add_argument("-v", "--verbose", action="count", default=0)
    return argp.parse_args()


def main() -> int:
    args = parse_args()
    configure_logging(verbosity_to_loglevel(args.verbose, args.quiet))

    # Obtain version information from Doomseeker binary
    platform = get_system_platform()
    doomseeker_exe = args.binary_dir / platform.doomseeker_exe
    version_info = obtain_version_information(doomseeker_exe)
    if not version_info:
        logger.error("No version information obtained from Doomseeker binary")
        return EC_ERROR
    logger.info("Version information:")
    for pkg in version_info:
        logger.info(
            "  %s: %s / %s / %s",
            pkg.name, pkg.display_name,
            pkg.display_version, pkg.revision
        )

    # Create the output directory
    output_dir = spawn_unique_dir(f"upkgs-{args.update_channel}")

    # Pack the actual packages from the version info
    packages_output_dir = output_dir / platform.name / args.update_channel
    logger.info("Packages output directory: %s", packages_output_dir)
    os.makedirs(packages_output_dir)

    successes: List[Package] = []
    failures: List[Tuple[Package, Exception]] = []
    for pkg in version_info:
        if pkg.name in PKG_BLACKLIST:
            logger.info("-- Skipping package: %s", pkg.name)
            continue
        try:
            process_package(pkg, platform,
                            args.update_channel,
                            args.binary_dir,
                            packages_output_dir)
            successes.append(pkg)
        except Exception as e:
            logger.exception("Error when packaging '%s': %s", pkg.name, e)
            failures.append((pkg, e))

    # Packaging summary
    for pkg in successes:
        logger.info("Success: %s", pkg.name)
    for pkg, error in failures:
        logger.info("FAILURE: %s: %s", pkg.name, error)

    if successes:
        logger.info("Created packages are in directory: %s", output_dir)
        logger.info("%s", output_dir.absolute())
        # If at least one package was successful, create the update-info file.
        update_info_path = (
            output_dir /
            f"update-info_{platform.name}_{args.update_channel}.js"
        )
        logger.info("Creating update info file: %s", update_info_path)
        with update_info_path.open("w") as update_info_io:
            dump_update_info(update_info_io, platform,
                             args.update_channel, version_info)
    else:
        # Delete the output directory if all packages failed.
        output_dir.rmdir()

    # Set the appropriate error code
    if not successes and not failures:
        # nothing happened?
        return EC_ERROR
    else:
        return EC_OK if not failures else EC_FAIL


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as e:
        logger.exception(e)
        sys.exit(EC_ERROR)
