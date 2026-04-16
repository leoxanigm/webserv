#!/usr/bin/env python3
import os
import sys
import html

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
print("<h1>Python CGI works</h1>")
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