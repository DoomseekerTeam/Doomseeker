# Odamex server reply fixtures

The `.bin` files contain raw UDP reply payloads captured from Odamex game
servers for use in parser tests.

Filenames follow `odamex_p<protocol-version>_<trait>.bin`:

- `odamex`: the game engine.
- `p<protocol-version>`: the query protocol version, such as `p7`.
- `<trait>`: a distinguishing trait of the captured reply.

Individual fixtures are described in comments in `resources.qrc`.
