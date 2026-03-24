#!/bin/bash

for i in $(seq 1 1000); do
  (
    printf 'GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n' | nc localhost 8080 > /dev/null
  ) &
done
wait