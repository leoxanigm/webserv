#!/usr/bin/env python3
import os
import sys
import html
import urllib.parse
from datetime import datetime

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
ENTRIES_DIR = os.path.join(BASE_DIR, "entries")

if not os.path.isdir(ENTRIES_DIR):
    os.makedirs(ENTRIES_DIR, 0o755)

def escape(s):
    return html.escape(s, quote=True)

def get_request_method():
    return os.environ.get("REQUEST_METHOD", "GET").upper()

def get_content_length():
    raw = os.environ.get("CONTENT_LENGTH", "")
    try:
        return int(raw)
    except (TypeError, ValueError):
        return 0

def read_request_body():
    length = get_content_length()
    if length <= 0:
        return ""
    return sys.stdin.read(length)

def parse_form_urlencoded():
    body = read_request_body()
    parsed = urllib.parse.parse_qs(body, keep_blank_values=True)
    result = {}
    for key, values in parsed.items():
        result[key] = values[0] if values else ""
    return result

def redirect(location):
    sys.stdout.write("Status: 303 See Other\r\n")
    sys.stdout.write("Location: {}\r\n".format(location))
    sys.stdout.write("\r\n")

def safe_entry_name(name):
    name = os.path.basename(name)
    allowed = []
    for ch in name:
        if ch.isalnum() or ch in ("_", "-", "."):
            allowed.append(ch)
    return "".join(allowed)

def list_entries():
    items = []
    for entry in os.listdir(ENTRIES_DIR):
        if entry in (".", ".."):
            continue
        full = os.path.join(ENTRIES_DIR, entry)
        if not os.path.isfile(full):
            continue
        if not entry.endswith(".txt"):
            continue

        try:
            with open(full, "r") as f:
                raw = f.read()
        except IOError:
            continue

        title = ""
        date = ""
        body = ""

        lines = raw.splitlines()
        mode = None
        body_lines = []

        for line in lines:
            if mode == "body":
                body_lines.append(line)
                continue

            if line.startswith("Title: "):
                title = line[len("Title: "):]
            elif line.startswith("Date: "):
                date = line[len("Date: "):]
            elif line == "Body:":
                mode = "body"

        body = "\n".join(body_lines)

        items.append({
            "id": entry,
            "title": title,
            "date": date,
            "body": body,
        })

    items.sort(key=lambda x: x["id"], reverse=True)
    return items

def save_entry(title, body):
    title = title.strip()
    body = body.strip()

    if not title or not body:
        return False

    now = datetime.utcnow()
    file_id = now.strftime("%Y%m%d%H%M%S%f") + ".txt"
    full = os.path.join(ENTRIES_DIR, file_id)

    content = []
    content.append("Title: {}".format(title))
    content.append("Date: {}".format(now.strftime("%Y-%m-%d %H:%M:%S UTC")))
    content.append("Body:")
    content.append(body)

    try:
        with open(full, "w") as f:
            f.write("\n".join(content))
    except IOError:
        return False

    os.chmod(full, 0o644)
    return True

def delete_entry(entry_id):
    entry_id = safe_entry_name(entry_id)
    full = os.path.join(ENTRIES_DIR, entry_id)

    if not entry_id.endswith(".txt"):
        return

    if os.path.isfile(full):
        try:
            os.remove(full)
        except OSError:
            pass

def handle_post():
    content_type = os.environ.get("CONTENT_TYPE", "")
    if not content_type.startswith("application/x-www-form-urlencoded"):
        redirect("/blog")
        return

    form = parse_form_urlencoded()
    action = form.get("action", "")

    if action == "delete":
        delete_entry(form.get("id", ""))
        redirect("/blog")
        return

    if action == "create":
        title = form.get("title", "")
        body = form.get("body", "")
        save_entry(title, body)
        redirect("/blog")
        return

    redirect("/blog")

def render_page():
    entries = list_entries()

    sys.stdout.write("Content-Type: text/html; charset=UTF-8\r\n")
    sys.stdout.write("\r\n")

    sys.stdout.write("""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>Blog</title>
<style>
body {
    font-family: Arial, sans-serif;
    margin: 40px;
    background: #f4f4f4;
    color: #222;
}
h1, h2 {
    margin-bottom: 16px;
}
.box {
    background: #fff;
    border: 1px solid #ddd;
    padding: 20px;
    margin-bottom: 24px;
}
input[type="text"], textarea {
    width: 100%;
    box-sizing: border-box;
    margin-bottom: 12px;
    padding: 8px;
    border: 1px solid #ccc;
}
textarea {
    min-height: 140px;
    resize: vertical;
}
.entry {
    border: 1px solid #ddd;
    background: #fff;
    padding: 16px;
    margin-bottom: 16px;
}
.meta {
    color: #666;
    font-size: 14px;
    margin-bottom: 10px;
}
pre {
    white-space: pre-wrap;
    word-wrap: break-word;
    font-family: inherit;
    margin: 0;
}
</style>
</head>
<body>
<h1>Blog</h1>

<div class="box">
    <h2>New entry</h2>
    <form action="/blog" method="POST">
        <input type="hidden" name="action" value="create">
        <label>Title</label>
        <input type="text" name="title" maxlength="200" required>
        <label>Body</label>
        <textarea name="body" required></textarea>
        <button type="submit">Post</button>
    </form>
</div>

<div class="box">
    <h2>Entries</h2>
""")

    if not entries:
        sys.stdout.write("<p>No entries yet.</p>\n")
    else:
        for entry in entries:
            sys.stdout.write('<div class="entry">\n')
            sys.stdout.write("<h3>{}</h3>\n".format(escape(entry["title"])))
            sys.stdout.write('<div class="meta">{}</div>\n'.format(escape(entry["date"])))
            sys.stdout.write("<pre>{}</pre>\n".format(escape(entry["body"])))
            sys.stdout.write("""
<form action="/blog" method="POST" style="margin-top: 12px;">
    <input type="hidden" name="action" value="delete">
    <input type="hidden" name="id" value="{}">
    <button type="submit">Delete</button>
</form>
""".format(escape(entry["id"])))
            sys.stdout.write("</div>\n")

    sys.stdout.write("""
</div>
</body>
</html>
""")

def main():
    method = get_request_method()

    if method == "POST":
        handle_post()
        return

    render_page()

if __name__ == "__main__":
    main()