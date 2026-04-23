*This project has been created as part of the 42 curriculum by mathboye and zalemu*

# Description

`webserv` is a non-blocking HTTP/1.1 server written in C++98. The project focuses on building the core pieces of a web server from scratch: socket setup, polling, request parsing, routing, response generation, error handling, static file serving, uploads, and CGI execution.

The server starts from a configuration file inspired by NGINX syntax. Each configured `server` block opens its own listening socket, and all client sockets plus CGI pipe file descriptors are handled through a single `poll()` loop.

## What this implementation supports

1. **Multiple listening sockets**
   - The default configuration starts two servers:
     - `0.0.0.0:8080`
     - `0.0.0.0:9090`

2. **Non-blocking I/O with `poll()`**
   - listening sockets
   - accepted client sockets
   - CGI stdin/stdout pipes
   - idle client timeout handling

3. **HTTP request parsing**
   - request line validation
   - header parsing and validation
   - `Host` handling for HTTP/1.1
   - `Content-Length`
   - `Transfer-Encoding: chunked`
   - request completeness detection

4. **Implemented methods**
   - `GET`
   - `POST`
   - `DELETE`

5. **Static file serving**
   - serves files from configured roots
   - MIME type detection from file extension
   - configurable index files
   - automatic directory listing when enabled

6. **Routing from configuration**
   - allowed methods per location
   - route root mapping
   - autoindex on/off
   - index file selection
   - redirection with `308`
   - per-server body size limit
   - per-server client timeout

7. **Uploads and deletion**
   - raw POST body storage
   - multipart/form-data file upload handling for non-CGI routes
   - file deletion through `DELETE`

8. **CGI support**
   - PHP via `php-cgi`
   - Python via `python3`
   - request body forwarding through stdin
   - CGI response header parsing (`Status`, `Location`, `Content-Type`, etc.)
   - EOF-based CGI output handling

9. **Default error handling**
   - built-in HTML pages under `confs/status_pages/`
   - currently used status codes include:
     - `400 Bad Request`
     - `403 Forbidden`
     - `404 Not Found`
     - `405 Method Not Allowed`
     - `406 Not Acceptable`
     - `408 Request Timeout`
     - `411 Length Required`
     - `413 Payload Too Large`
     - `414 URI Too Long`
     - `415 Unsupported Media Type`
     - `416 Range Not Satisfiable`
     - `417 Expectation Failed`
     - `500 Internal Server Error`
     - `501 Not Implemented`
     - `502 Bad Gateway`
     - `504 Gateway Timeout`
     - `505 HTTP Version Not Supported`

## Project structure

```text
.
├── confs/
│   ├── default.conf
│   └── status_pages/
├── includes/
│   ├── parsing/
│   ├── request/
│   └── response/
├── srcs/
│   ├── parsing/
│   ├── request/
│   └── response/
├── test/
├── ressources/
└── www/
    └── html/
```

## Core components

### Parsing
- `ConfigFileParsing`: parses the NGINX-like configuration file into server and location structures.
- `HTTPRequestParser`: parses raw HTTP requests and exposes method, path, headers, body, content type, and completeness state.

### Request/event layer
- `Socket`: creates and configures listening sockets.
- `Connection`: owns the `poll()` loop, sockets, connected clients, and CGI pipe registration.
- `Client`: owns per-connection state, request buffer, response buffer, CGI state, timeout tracking, and state transitions.

### Response layer
- `ResponseBuilder`: resolves the matching location and builds static, upload, delete, redirect, or CGI-backed responses.
- `HTTPResponse`: serializes the final HTTP response.
- `CGIHandler`: forks the CGI process, wires pipes, builds CGI environment variables, and manages child execution.

## Routes in the default configuration

### Port 8080

- `/`
  - root: `./www/html`
  - methods: `GET`
  - serves `index.html`

- `/files`
  - root: `./www/html/files`
  - methods: `GET POST DELETE`
  - autoindex: `on`
  - useful for testing upload/list/delete on regular files

- `/no_files`
  - root: `./www/html/files`
  - methods: `GET POST DELETE`
  - autoindex disabled
  - directory access returns `403 Forbidden`

- `/gallery`
  - root: `./www/html/gallery`
  - methods: `GET POST`
  - CGI: PHP (`/usr/bin/php-cgi`)
  - serves `index.php`
  - demo gallery with image upload, listing, and deletion

- `/blog`
  - root: `./www/html/blog`
  - methods: `GET POST`
  - CGI: Python (`/usr/bin/python3`)
  - serves `index.py`
  - demo blog with entry creation, listing, and deletion

- `/admin`
  - `308` redirect to `/`

- `/cookie-clicker`
  - root: `./www/html/cookie-clicker`
  - methods: `GET POST`
  - CGI: Python (`/usr/bin/python3`)
  - serves `cookie_clicker.py`

### Port 9090

- `/`
  - example second server block listening on a different port

# Instructions

## Compilation

Build the executable with:

```bash
make
```

Clean object files:

```bash
make clean
```

Remove objects and executable:

```bash
make fclean
```

Rebuild from scratch:

```bash
make re
```

## Installation

### Required tools

- C++ compiler with C++98 support
- `make`
- `php-cgi`
- `python3`
- `nc` / netcat for manual tests
- a browser or `curl` for runtime checks

### Example Debian/Ubuntu packages

```bash
sudo apt update
sudo apt install build-essential php-cgi python3 netcat-openbsd curl
```

## Execution

Run with the default configuration:

```bash
./webserv
```

Run with an explicit configuration file:

```bash
./webserv confs/default.conf
```

When the server starts, it prints the configured routes and the addresses where they are available.

Stop the server with:

```bash
Ctrl+C
```

## Quick manual checks

Open in a browser:

```text
http://localhost:8080/
http://localhost:8080/files
http://localhost:8080/gallery
http://localhost:8080/blog
http://localhost:8080/cookie-clicker
```

### Static file request

```bash
curl -i http://localhost:8080/
```

### Directory listing

```bash
curl -i http://localhost:8080/files/
```

### Upload a file to `/files`

```bash
curl -i -X POST -F "file=@./www/html/files/vees.png" http://localhost:8080/files/
```

### Delete a file from `/files`

```bash
curl -i -X DELETE http://localhost:8080/files/vees.png
```

### Open the PHP gallery demo

```bash
curl -i http://localhost:8080/gallery
```

### Open the Python blog demo

```bash
curl -i http://localhost:8080/blog
```

## Provided test helpers

The repository includes several small scripts under `test/`:

- `bad_requests.sh` — malformed request checks
- `chuncked_request.sh` — chunked POST test
- `large_body.sh` — oversized body test
- `slow_partial.sh` — timeout behavior with partial request
- `stress_test.sh` — many concurrent GET requests
- `requests_sender.sh` — assorted manual request examples

Example:

```bash
bash test/bad_requests.sh
bash test/stress_test.sh
```

# Resources

- RFC 7230 — Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing
- RFC 7231 — Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content
- RFC 3875 — The Common Gateway Interface (CGI) Version 1.1
- MDN Web Docs — HTTP overview and status codes
- Beej’s Guide to Network Programming
- `man 2 socket`, `man 2 poll`, `man 2 accept`, `man 2 recv`, `man 2 send`, `man 2 fork`, `man 2 waitpid`, `man 3 getaddrinfo`
- NGINX documentation, used as behavioral reference for routing ideas and response comparisons

## AI Usage

AI was used as a support tool for:

- reviewing CGI edge cases
- checking `poll()`-driven state transitions
- discussing HTTP parsing and error-handling behavior
- reviewing multipart/form-data handling
- drafting and polishing documentation

All generated suggestions were manually reviewed, adapted to the project constraints, and tested against the running server.
