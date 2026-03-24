#!/bin/bash

{
  printf 'POST /upload HTTP/1.1\r\n'
  printf 'Host: localhost\r\n'
  printf 'Content-Length: 100000\r\n'
  printf 'Connection: close\r\n'
  printf '\r\n'
  head -c 100000 /dev/zero
} | nc localhost 8080 > /dev/null