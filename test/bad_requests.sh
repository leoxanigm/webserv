#!/bin/bash

echo "=== Bad GET request 1 ==="

{
	printf '/ HTTP/1.1\r\n'
	printf 'Host: localhost\r\n'
	printf 'Accept: text/html\r\n'
	printf '\r\n'
} | nc localhost 8080

echo "=== Bad GET request 2 ==="

{
	printf 'GET  HTTP/1.1\r\n'
	printf 'Host: localhost\r\n'
	printf 'Accept: text/html\r\n'
	printf '\r\n'
} | nc localhost 8080

echo "=== Bad GET request 3 ==="

{
	printf 'GET / \r\n'
	printf 'Host: localhost\r\n'
	printf 'Accept: text/html\r\n'
	printf '\r\n'
} | nc localhost 8080

echo "=== Bad GET request 4 ==="

{
	printf 'GET / HTTP/2.0\r\n'
	printf 'Host: localhost\r\n'
	printf 'Accept: text/html\r\n'
	printf '\r\n'
} | nc localhost 8080

echo "=== Bad GET request 5 ==="

{
	printf 'GET / HTTP/1.1\r\n'
	# printf 'Host: localhost\r\n'
	printf 'Accept: text/html\r\n'
	printf '\r\n'
} | nc localhost 8080

echo "=== Unsupported request 6 ==="

{
	printf 'PUT / HTTP/1.1\r\n'
	# printf 'Host: localhost\r\n'
	printf 'Accept: text/html\r\n'
	printf '\r\n'
} | nc localhost 8080

# # Incomplete POST request 1
# echo "=== Incomplete POST request 1 ==="

# {
# 	printf 'POST / HTTP/1.1\r\n'
# 	printf 'Host: localhost\r\n'
# 	printf 'Accept/html: text/html\r\n'
# 	printf 'Transfer-Encoding: chunked\r\n'
# 	# printf '\r\n'
# 	printf '7\r\nWelcome\r\n1c\r\nto Mozilla Developer Network\r\n'
# 	printf '\r0\n\r\n'
# } | nc localhost 8080


# # Incomplete GET request 2
# echo "=== Incomplete GET request 2 ==="

# {
# 	printf 'GET / HTTP/1.1\r\n'
# 	printf 'Host: localhost\r\n'
# 	printf 'Accept/html: text/html'
# 	printf '\r\n'
# } | nc localhost 8080

# # Incomplete POST request 2
# echo "=== Incomplete POST request 2 ==="

# {
# 	printf 'POST / HTTP/1.1\r\n'
# 	printf 'Host: localhost\r\n'
# 	printf 'Accept/html: text/html\r\n'
# 	printf 'Transfer-Encoding: chunked\r\n'
# 	printf '\r\n'
# 	printf '7\r\nWelcome\r\n1c\r\nto Mozilla Developer Network\r\n'
# 	printf '\r\n\r\n'
# } | nc localhost 8080
