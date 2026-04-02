#!/bin/bash

printf 'GET /index.html HTTP/1.1\r\nHost: localhost\r\nAccept: \r\n\r\n' | nc localhost 8080

# Valid GET request
echo "=== Valid GET request ==="
{
	printf 'GET / HTTP/1.1\r\n'
	printf 'Host: localhost\r\n'
	printf 'Accept/html: text/html\r\n'
	printf '\r\n'
} | nc localhost 8080

# Valid POST request
echo "=== Valid POST request ==="

# {
# 	printf 'POST / HTTP/1.1\r\n'
# 	printf 'Host: localhost\r\n'
# 	printf 'Accept/html: text/html\r\n'
# 	printf 'Transfer-Encoding: chunked\r\n'
# 	printf '\r\n'
# 	printf '7\r\nWelcome\r\n1c\r\nto Mozilla Developer Network\r\n'
# 	printf '\r0\n\r\n'
# } | nc localhost 8080


# Incomplete GET request 1
echo "=== Incomplete GET request 1 ==="

{
	printf 'GET / HTTP/1.1\r\n'
	printf 'Host: localhost\r\n'
	printf 'Accept/html: text/html\r\n'
	# printf '\r\n'
} | nc localhost 8080

# Incomplete POST request 1
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


# Incomplete GET request 2
echo "=== Incomplete GET request 2 ==="

{
	printf 'GET / HTTP/1.1\r\n'
	printf 'Host: localhost\r\n'
	printf 'Accept/html: text/html'
	printf '\r\n'
} | nc localhost 8080

# Incomplete POST request 2
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
