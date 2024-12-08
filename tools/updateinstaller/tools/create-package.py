#!/usr/bin/python3
#
# ------------------------------------------------------------------------------
# create-package.py
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
"""Takes the set of files that make up a release and puts them into a
.zip package along with an .xml file listing all the files in the
release and mapping them to the package.

This script is a part of Doomseeker's update packages auto-build system.

Outputs:

 <output-dir>/$PACKAGE_NAME.zip
   This is a package containing the files in this version of the software.

 <output-dir>/$PACKAGE_NAME.zip.xml
   This file lists all the files contained in this version of the software
   and the package which they are contained in.
"""
import argparse
import hashlib
import json
import logging
import os
import sys
import xml.dom.minidom as minidom
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from io import StringIO
from pathlib import Path, PurePath
from typing import Dict, List, TextIO, Tuple
from zipfile import ZipFile, ZIP_DEFLATED

logger = logging.getLogger(__name__)

EC_OK = 0
EC_ERROR = 1


@dataclass
class PackageConfig:
    name: str
    main_binary: str
    updater_binary: str
    files: List[PurePath]

    @classmethod
    def from_config(cls, config: dict):
        return cls(
            name=config["name"],
            main_binary=config["main-binary"],
            updater_binary=config["updater-binary"],
            files=[PurePath(f) for f in config.get("files", [])],
        )

    def is_main_binary(self, arcpath: PurePath):
        return str(arcpath) == self.main_binary

    def matches_file(self, arcpath: PurePath):
        return arcpath in self.files


PackageFile = Tuple[PurePath, Path]


def file_sha1(file: Path) -> str:
    digest = hashlib.sha1()
    with file.open("rb") as f:
        while True:
            chunk = f.read(4 * 1024 * 1024)
            if not chunk:
                break
            digest.update(chunk)
    return digest.hexdigest()


def dump_metadata_xml(output: TextIO, version: str, platform: str,
                      package_file: Path, package_file_list: List[PackageFile],
                      package_config: PackageConfig):
    def add_elem(parent: ET.Element, tag: str, text: str = None,
                 attrib: Dict[str, str] = None):
        elem = ET.SubElement(parent, tag, attrib or {})
        if text is not None:
            elem.text = text
        return elem

    def filemode_str(bits: int) -> str:
        return "0" + oct(bits & 0o777)[2:]

    root = ET.Element("update", attrib={"version": "3"})

    # 'update' (root)
    add_elem(root, "targetVersion", version)
    add_elem(root, "platform", platform)

    # 'dependencies'
    deps_elem = add_elem(root, "dependencies")
    add_elem(deps_elem, "file", package_config.updater_binary)

    # 'packages' (there's only one)
    packages_elem = add_elem(root, "packages")
    package_elem = add_elem(packages_elem, "package")
    add_elem(package_elem, "name", package_config.name)
    add_elem(package_elem, "size", str(package_file.stat().st_size))
    add_elem(package_elem, "hash", file_sha1(package_file))

    # 'install'
    install_elem = add_elem(root, "install")
    for arcpath, filepath in package_file_list:
        file_elem = add_elem(install_elem, "file")
        add_elem(file_elem, "name", arcpath.as_posix())
        if filepath.is_symlink():
            add_elem(file_elem, "target", os.readlink(filepath))
        else:
            add_elem(file_elem, "size", str(filepath.stat().st_size))
            add_elem(file_elem, "permissions",
                     filemode_str(filepath.stat().st_mode))
            add_elem(file_elem, "hash", file_sha1(filepath))
            add_elem(file_elem, "package", package_config.name)
        if package_config.is_main_binary(arcpath):
            add_elem(file_elem, "is-main-binary", "true")

    # run after
    add_elem(root, "run-after-install", package_config.main_binary)

    # Write it out.
    tree = ET.ElementTree(root)
    # ET.indent() is in Python 3.9, but let's stay compatible with older.
    with StringIO() as dumpbuf:
        tree.write(dumpbuf, encoding="unicode", xml_declaration=True)
        dumpbuf.seek(0)
        pretty_xml = minidom.parse(dumpbuf).toprettyxml()
    output.write(pretty_xml)


def reraise(e: Exception):
    raise e


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
    argp.add_argument("--overwrite", action='store_true', default=False,
                      help="overwrite the archives if they exist already")
    argp.add_argument("-p", "--platform", required=True,
                      help="the target platform for the update")
    argp.add_argument("-s", "--suffix", default="",
                      help="suffix added to the package and script filenames")
    argp.add_argument("-V", "--version", required=True,
                      help="the target version string for the update")
    argp.add_argument("-q", "--quiet", action="count", default=0)
    argp.add_argument("-v", "--verbose", action="count", default=0)
    argp.add_argument("input_dir", type=Path,
                      help="where are the files to collect")
    argp.add_argument("package_map_file", type=argparse.FileType("r"),
                      help="packaging config JSON file")
    argp.add_argument("output_dir", type=Path,
                      help="where to save the packaged files")
    return argp.parse_args()


def main() -> int:
    args = parse_args()
    configure_logging(verbosity_to_loglevel(args.verbose, args.quiet))

    # Args validation.
    if not args.input_dir.is_dir():
        logger.error("Input directory does not exist: %s", args.input_dir)
        return EC_ERROR
    if not args.output_dir.is_dir():
        logger.error("Output directory does not exist: %s", args.output_dir)
        return EC_ERROR

    # Read the package config.
    package_config = PackageConfig.from_config(
        json.load(args.package_map_file))
    logger.info("Loaded package config: %s", package_config.name)

    archive_filename = f"{package_config.name}{args.suffix}.zip"
    archive_filepath = args.output_dir / archive_filename
    metadata_filepath = Path(f"{archive_filepath}.xml")
    if archive_filepath.exists() and not args.overwrite:
        logger.error("Package archive already exists, aborting: %s",
                     archive_filepath)
        return EC_ERROR
    if metadata_filepath.exists() and not args.overwrite:
        logger.error("Package archive metadata already exists, aborting: %s",
                     metadata_filepath)
        return EC_ERROR

    # Collect the files that go into the package.
    logger.info("Collecting files: %s ...", args.input_dir)
    package_file_list: List[PackageFile] = []
    # Path.walk() is in Python 3.12; let's stay compatible with older
    # Pythons for now.
    for dirpath, dirnames, filenames in os.walk(args.input_dir,
                                                onerror=reraise):
        arcpath = PurePath(Path(dirpath).relative_to(args.input_dir))
        logger.debug("  walk: %s", arcpath)
        for file in filenames:
            if package_config.matches_file(arcpath / file):
                logger.info("    %s", arcpath / file)
                package_file_list.append((
                    arcpath / file,
                    Path(dirpath) / file,
                ))

    # Validate the collected files.
    if not package_file_list:
        logger.error("Empty package, aborting")
        return EC_ERROR

    for arcpath, _ in package_file_list:
        if ".." in arcpath.parts:
            logger.error("something went wrong, '..' should "
                         "not appear in an archive path: %s", arcpath)
            return EC_ERROR
        if arcpath.is_absolute():
            logger.error("something went wrong, archive should "
                         "not have absolute paths: %s", arcpath)
            return EC_ERROR

    # Sort the files by arcpath to ensure stable packaging results. When the
    # files don't change, the resulting package should always be the same.
    #
    # The paths are converted to str type (e[0].as_posix()) so that character
    # case is taken into account when sorting, just like it was in previous
    # versions of this tool.
    package_file_list = list(sorted(
        package_file_list,
        key=lambda e: e[0].as_posix(),
    ))

    # Create the archive.
    logger.info("Creating archive: %s", archive_filepath)
    with ZipFile(archive_filepath, 'w',
                 compression=ZIP_DEFLATED, compresslevel=9,
                 allowZip64=False) as zipfile:
        for arcpath, filepath in package_file_list:
            logger.info("  Packaging: %s as %s", filepath, arcpath)
            zipfile.write(filepath, arcpath)

    # Create the metadata XML.
    with metadata_filepath.open("w") as metadata_file:
        dump_metadata_xml(metadata_file, args.version, args.platform,
                          archive_filepath, package_file_list,
                          package_config)

    return EC_OK


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as e:
        logger.exception(e)
        sys.exit(EC_ERROR)
