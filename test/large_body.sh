#!/bin/bash

{
  printf 'POST /upload HTTP/1.1\r\n'
  printf 'Host: localhost\r\n'
  # printf 'Content-Length: 100000\r\n'
  printf 'Content-Length: 20\r\n'
  printf 'Connection: close\r\n'
  printf '\r\n'
  # head -c 100000 /dev/random
  printf 'test\r\n'
  printf 'test\r\n'
  printf 'test\r\n'
  printf 'test\r\n'
  printf 'test\r\n'
  # printf '\r\n\r\n'
} | nc localhost 8080 > /dev/null