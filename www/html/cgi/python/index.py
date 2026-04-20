#!/usr/bin/env python3
import os
import sys
import html
import time

print("Content-Type: text/html")
print()

method = os.environ.get("REQUEST_METHOD", "")
query = os.environ.get("QUERY_STRING", "")
content_length = int(os.environ.get("CONTENT_LENGTH", "0") or "0")
body = sys.stdin.read(content_length) if content_length > 0 else ""

print("<!doctype html>")
print("<html>")
print("<head><title>Python CGI Test</title></head>")
print("<body>")

# test timeout (server should send status 500)
# time.sleep(60)

print("<h1>Python CGI works</h1>")

# # test big content (server should send status 500)
# for i in range(1000000):
#     print("<p>You shouldn't see me.</p>")


print("<p><strong>REQUEST_METHOD:</strong> {}</p>".format(html.escape(method)))
print("<p><strong>QUERY_STRING:</strong> {}</p>".format(html.escape(query)))
print("<p><strong>BODY:</strong> {}</p>".format(html.escape(body)))
print("<h2>Environment</h2>")
print("<pre>")
for key in sorted(os.environ):
    print("{}={}".format(html.escape(key), html.escape(os.environ[key])))
print("</pre>")
print("</body>")
print("</html>")