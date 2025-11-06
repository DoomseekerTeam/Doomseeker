import shlex
import subprocess
import sys
import tempfile
from enum import IntEnum
from pathlib import Path


class WadseekerApp:
    class EC(IntEnum):
        OK = 0
        ARGS = 1
        FAIL = 2

    def __init__(self, exe: Path, outputdir: str = tempfile.gettempdir()):
        self.exe = exe
        self.outputdir = outputdir

    def run_with_site(
        self, url: list[str] | str,
        files: list[str] | str,
        timeout: float = None,
    ) -> (int, str):
        if not isinstance(url, list):
            url = [url]
        if not isinstance(files, list):
            files = [files]
        site_args = list(zip(["-u"] * len(url), url))
        site_args = [item for pair in site_args for item in pair]
        return self._run(
            options=["--no-default-sites", "--no-idgames"] + site_args,
            files=files
        )

    def _run(
        self, options: list[str] = None,
        files: list[str] = None,
        timeout: float = None,
    ) -> (int, str):
        options = options or []
        files = files or []
        cmd = [self.exe.absolute()] + options + files
        cmd_quoted = [shlex.quote(str(arg)) for arg in cmd]
        print("Popen:", " ".join(cmd_quoted), file=sys.stderr)
        with tempfile.TemporaryFile() as out:
            process = subprocess.Popen(
                cmd,
                cwd=self.outputdir,
                stdout=out,
                stderr=subprocess.STDOUT,
            )
            try:
                exitcode = process.wait(timeout=timeout)
            except subprocess.TimeoutExpired:
                process.kill()
                exitcode = process.wait()
            out.flush()
            out.seek(0)
            output = out.read().decode("utf-8", errors="replace")
            print(output, file=sys.stderr)
        return (exitcode, output)
