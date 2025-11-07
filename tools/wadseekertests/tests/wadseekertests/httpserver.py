import errno
import socketserver
import threading
import http.server
from pathlib import Path

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
