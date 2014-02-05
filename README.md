# Inanity Oil

Inanity Oil is an experimental real-time collaborative editor of structured data.

## Goal

Provide foundation for data-driven game engine editor, with possibility of easy writing custom data processors, live collaboration support, and without using of traditional VCS.

Inanity Oil is developing as game data editor for [Inanity](https://github.com/quyse/inanity) engine.

## Status

Early work-in-progress.

## Tech

* [The Mozilla platform](https://developer.mozilla.org/en-US/docs/The_Mozilla_platform) as basis for client.
* [Inanity](https://github.com/quyse/inanity)-based NPAPI plugin for client's native operation and graphics/sound support.
* [SQLite](https://sqlite.org) databases as a backend for storing client and server repositories.
* [FastCGI](http://www.fastcgi.com/) as server's interface to network.
