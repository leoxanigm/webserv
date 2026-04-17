#!/bin/bash

{
  printf 'POST /upload HTTP/1.1\r\n'
  printf 'Host: localhost\r\n'
  printf 'Content-Type: application/octet-stream\r\n'
  printf 'Content-Length: 1000000\r\n'
  printf 'Connection: close\r\n'
  printf '\r\n'
  head -c 1000000 /dev/zero
} | nc localhost 8080