#!/bin/bash

{
  printf 'GET / HTTP/1.1\r\n'
  sleep 1
  printf 'Host: localhost\r\n'
  sleep 1
  printf 'Connection: close\r\n'
  sleep 1
  printf '\r\n'
} | nc localhost 8080