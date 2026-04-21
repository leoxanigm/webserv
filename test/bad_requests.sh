#!/bin/bash

# Function to send a request over netcat and check the response status code
send_request() {
	local name="$1"
	local expected_code="$2"
	shift 2
	echo -n "=== $name ... "
	
	local response=$(
		{
			for arg in "$@"; do
				printf "$arg\r\n"
			done
			sleep 0.1
		} | nc localhost 8080 2>/dev/null
	)
	
	# Extract the HTTP status code from the first line
	local actual_code=$(echo "$response" | head -n 1 | awk '{print $2}' | tr -d '\r')
	
	if [ "$actual_code" = "$expected_code" ]; then
		echo -e "\033[32mOK ($actual_code)\033[0m"
	else
		echo -e "\033[31mFAIL (Expected: $expected_code, Got: '$actual_code')\033[0m"
	fi
}

send_request "Bad GET request 1" "400" \
	"/ HTTP/1.1" \
	"Host: localhost" \
	"Accept: text/html" \
	""

send_request "Bad GET request 2" "400" \
	"GET  HTTP/1.1" \
	"Host: localhost" \
	"Accept: text/html" \
	""

send_request "Bad GET request 3" "400" \
	"GET / " \
	"Host: localhost" \
	"Accept: text/html" \
	""

send_request "Bad GET request 4" "505" \
	"GET / HTTP/2.0" \
	"Host: localhost" \
	"Accept: text/html" \
	""

send_request "Bad GET request 5" "400" \
	"GET / HTTP/1.1" \
	"Accept: text/html" \
	""

send_request "Unsupported request 6" "501" \
	"PUT / HTTP/1.1" \
	"Accept: text/html" \
	""

send_request "Missing Host header HTTP/1.1" "400" \
	"GET / HTTP/1.1" \
	"Accept: text/html" \
	""

send_request "Invalid method" "501" \
	"INVALIDMETHOD / HTTP/1.1" \
	"Host: localhost" \
	"Accept: text/html" \
	""

send_request "Invalid URI (no slash)" "400" \
	"GET index.html HTTP/1.1" \
	"Host: localhost" \
	"Accept: text/html" \
	""

send_request "Long URI" "414" \
	"GET /$(printf 'a%.0s' {1..9000}) HTTP/1.1" \
	"Host: localhost" \
	"Accept: text/html" \
	""

send_request "Content-Length but no body" "400" \
	"POST / HTTP/1.1" \
	"Host: localhost" \
	"Content-Length: 100" \
	"" \
	"too short body"

send_request "Multiple Host headers" "400" \
	"GET / HTTP/1.1" \
	"Host: localhost" \
	"Host: 127.0.0.1" \
	""

send_request "Bad header syntax (no colon)" "400" \
	"GET / HTTP/1.1" \
	"Host: localhost" \
	"Accept text/html" \
	""

# Incomplete POST request 1
send_request "Incomplete POST request 1" "400" \
	"POST / HTTP/1.1" \
	"Host: localhost" \
	"Accept/html: text/html" \
	"Transfer-Encoding: chunked" \
	"7\r\nWelcome\r\n1c\r\nto Mozilla Developer Network\r\n\r0\n" \
	""

# Incomplete GET request 2
send_request "Incomplete GET request 2" "400" \
	"GET / HTTP/1.1" \
	"Host: localhost" \
	"Accept/html: text/html"

# Incomplete POST request 2
send_request "Incomplete POST request 2" "400" \
	"POST / HTTP/1.1" \
	"Host: localhost" \
	"Accept/html: text/html" \
	"Transfer-Encoding: chunked" \
	"" \
	"7\r\nWelcome\r\n1c\r\nto Mozilla Developer Network\r\n" \
	""
