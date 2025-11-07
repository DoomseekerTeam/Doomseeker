import errno
import mimetypes
import socketserver
import threading
import http.server
from http import HTTPStatus
from pathlib import Path
from urllib.parse import urlparse, parse_qs

import pytest

PORT_RANGE = (8000, 9000)


class HTTPHandler(http.server.SimpleHTTPRequestHandler):
    directory: Path = None

    def __init__(self, *args, **kwargs):
        super().__init__(
            *args,
            directory=str(self.directory) if self.directory else None,
            **kwargs,
        )

    def do_GET(self):
        parsed_url = urlparse(self.path)
        query = parse_qs(parsed_url.query)
        if parsed_url.path == "/attachment":
            return self.serve_attachment(query)
        else:
            return super().do_GET()

    def serve_attachment(self, query: dict[str, list[str]]):
        requested_file = query.get("file")[0]
        if not requested_file:
            return self.send_error(HTTPStatus.BAD_REQUEST)
        # Safety check: the file must exist in the directory,
        # but let's be case-insensitive here.
        available_files = [e.name for e in Path(self.directory).iterdir()
                           if e.is_file()]
        requested_file = _find_str_case_insensitive(
            available_files,
            requested_file,
        )
        # Check if we had a match.
        if requested_file is None:
            return self.send_error(HTTPStatus.NOT_FOUND)
        # Reconstruct path to the file for case-sensitive file systems.
        filepath = Path(self.directory) / requested_file
        # Send file now.
        self.send_response(HTTPStatus.OK)
        content_type = mimetypes.guess_type(filepath)[0] or "application/octet-stream"
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Disposition", f'attachment; filename="{requested_file}"')
        self.send_header("Content-Length", str(filepath.stat().st_size))
        self.end_headers()
        with filepath.open("rb") as file:
            self.copyfile(file, self.wfile)


def _find_str_case_insensitive(haystack: list[str], needle: str) -> str:
    for candidate in haystack:
        if candidate.casefold() == needle.casefold():
            return candidate
    return None


@pytest.fixture(scope="session")
def httpserver(request: pytest.FixtureRequest) -> str:
    """Launch a HTTP server with test data and return a URL to it.

    The server is bound to 127.0.0.1, and the port is auto-selected.
    """
    host = "127.0.0.1"

    # Get the path to the directory with static HTTP test data.
    http_static_dir: Path = request.config.rootpath / "data/http"
    if not http_static_dir.exists():
        raise FileNotFoundError(str(http_static_dir))

    class _ScopedHTTPHandler(HTTPHandler):
        directory = http_static_dir

    # Find the first free port.
    for port in range(*PORT_RANGE):
        try:
            httpserver = socketserver.TCPServer((host, port), _ScopedHTTPHandler)
            break  # no exception, free port found
        except OSError as e:
            if e.errno != errno.EADDRINUSE:
                raise

    # Serve forever in a background thread.
    thread = None
    try:
        thread = threading.Thread(target=httpserver.serve_forever)
        thread.start()
        yield f"http://{host}:{port}"
        httpserver.shutdown()
    finally:
        httpserver.server_close()
        if thread:
            thread.join()
