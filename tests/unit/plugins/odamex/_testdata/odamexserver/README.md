# Odamex server reply fixtures

The `.bin` files contain raw Odamex game-server UDP reply payloads for parser
tests. They include original captures and focused variants derived from them.

Filenames follow `odamex_p<protocol-version>_<trait>.bin`:

- `odamex`: the game engine.
- `p<protocol-version>`: the query protocol version, such as `p7`.
- `<trait>`: a distinguishing trait of the captured reply.

Individual fixtures are described in comments in `resources.qrc`.
