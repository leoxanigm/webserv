#!/bin/bash

echo "=== Valid chunked request ==="
{
  printf 'POST /files/chunked_text.txt HTTP/1.1\r\n'
  printf 'Host: localhost\r\n'
  printf 'Transfer-Encoding: chunked\r\n'
  printf '\r\n'
  printf '7\r\nDecoded\r\n4\r\nbody\r\n0\r\n\r\n'
} | nc localhost 8080