#!/usr/bin/env python3
import os
import sys

def parse_cookies(cookie_str):
    cookies = {}
    if not cookie_str:
        return cookies
    for part in cookie_str.split(';'):
        part = part.strip()
        if '=' in part:
            k, v = part.split('=', 1)
            cookies[k.strip()] = v.strip()
    return cookies

method = os.environ.get('REQUEST_METHOD', 'GET')
http_cookie = os.environ.get('HTTP_COOKIE', '')
cookies = parse_cookies(http_cookie)

count = 0
try:
    count = int(cookies.get('click_count', '0'))
except (ValueError, TypeError):
    count = 0

if method == 'POST':
    count += 1
    # Post/Redirect/Get: redirect so browser refresh doesn't resubmit the POST
    print("Status: 303 See Other")
    print("Location: /cookie-clicker")
    print("Set-Cookie: click_count={}; Path=/".format(count))
    print()
    raise SystemExit

print("Content-Type: text/html")
print("Set-Cookie: click_count={}; Path=/".format(count))
print()

print("""<!doctype html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>WEBSERV // COOKIE CLICKER</title>
    <style>
      @import url('https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=Oxanium:wght@300;600;800&display=swap');

      :root {
        --bg: #080c10;
        --surface: #0d1318;
        --border: #1e2d3a;
        --accent: #00e5c8;
        --accent2: #ff5f3e;
        --dim: #3a5060;
        --text: #c8dde8;
        --text-dim: #4a6070;
        --green: #39ff86;
        --amber: #ffb347;
        --red: #ff4444;
      }

      *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

      body {
        background: var(--bg);
        color: var(--text);
        font-family: 'Share Tech Mono', monospace;
        font-size: 14px;
        min-height: 100vh;
        display: flex;
        flex-direction: column;
        overflow-x: hidden;
      }

      body::before {
        content: '';
        position: fixed;
        inset: 0;
        background: repeating-linear-gradient(
          0deg, transparent, transparent 2px,
          rgba(0,0,0,0.07) 2px, rgba(0,0,0,0.07) 4px
        );
        pointer-events: none;
        z-index: 9999;
      }

      header {
        border-bottom: 1px solid var(--border);
        padding: 32px 40px 24px;
        display: flex;
        align-items: flex-end;
        justify-content: space-between;
        gap: 24px;
      }

      .logo {
        font-family: 'Oxanium', sans-serif;
        font-weight: 800;
        font-size: 28px;
        letter-spacing: 0.12em;
        color: var(--accent);
        text-shadow: 0 0 20px rgba(0,229,200,0.4);
      }
      .logo span { color: var(--accent2); }

      main {
        flex: 1;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        padding: 60px 40px;
        gap: 32px;
      }

      .card {
        background: var(--surface);
        border: 1px solid var(--border);
        padding: 48px 60px;
        text-align: center;
        position: relative;
        max-width: 480px;
        width: 100%;
        animation: fadeUp 0.5s ease both;
      }
      .card::before {
        content: '';
        position: absolute;
        top: 0; left: 0;
        width: 3px; height: 100%;
        background: var(--accent);
      }

      .card-label {
        font-family: 'Oxanium', sans-serif;
        font-weight: 600;
        font-size: 10px;
        letter-spacing: 0.2em;
        color: var(--text-dim);
        text-transform: uppercase;
        margin-bottom: 4px;
      }

      .count-display {
        font-family: 'Oxanium', sans-serif;
        font-weight: 800;
        font-size: 96px;
        color: var(--accent);
        text-shadow: 0 0 40px rgba(0,229,200,0.5);
        line-height: 1;
        margin: 24px 0 10px;
        letter-spacing: -0.02em;
      }

      .count-label {
        font-size: 11px;
        color: var(--text-dim);
        letter-spacing: 0.2em;
        text-transform: uppercase;
        margin-bottom: 40px;
      }

      .click-btn {
        font-family: 'Oxanium', sans-serif;
        font-weight: 800;
        font-size: 13px;
        letter-spacing: 0.2em;
        padding: 16px 48px;
        border: none;
        cursor: pointer;
        text-transform: uppercase;
        background: var(--accent);
        color: var(--bg);
        transition: all 0.15s;
        width: 100%;
      }
      .click-btn:hover {
        background: #00ffde;
        box-shadow: 0 0 32px rgba(0,229,200,0.5);
      }
      .click-btn:active { transform: scale(0.98); }

      .cookie-info {
        font-size: 11px;
        color: var(--text-dim);
        margin-top: 20px;
        letter-spacing: 0.04em;
        line-height: 1.8;
      }

      code {
        background: rgba(0,229,200,0.08);
        color: var(--accent);
        padding: 1px 5px;
        font-family: 'Share Tech Mono', monospace;
        font-size: 12px;
      }

      .btn-ghost {
        display: inline-block;
        font-family: 'Oxanium', sans-serif;
        font-weight: 600;
        font-size: 12px;
        letter-spacing: 0.15em;
        padding: 9px 20px;
        border: 1px solid var(--border);
        color: var(--text);
        text-decoration: none;
        text-transform: uppercase;
        transition: all 0.15s;
      }
      .btn-ghost:hover {
        border-color: var(--accent);
        color: var(--accent);
      }

      footer {
        border-top: 1px solid var(--border);
        padding: 20px 40px;
        display: flex;
        justify-content: space-between;
        align-items: center;
        font-size: 11px;
        color: var(--text-dim);
      }

      @keyframes fadeUp {
        from { opacity: 0; transform: translateY(12px); }
        to   { opacity: 1; transform: translateY(0); }
      }

      @media (max-width: 700px) {
        header { flex-direction: column; align-items: flex-start; padding: 20px; }
        main { padding: 40px 20px; }
        .card { padding: 32px 24px; }
        .count-display { font-size: 72px; }
      }
    </style>
  </head>
  <body>
    <header>
      <div>
        <div class="logo">WEBSERV <span>//</span> COOKIE CLICKER</div>
        <div style="font-size: 11px; color: var(--text-dim); margin-top: 4px">
          Cookie-persisted click counter &mdash; CGI/1.1
        </div>
      </div>
      <a href="/" class="btn-ghost">&larr; Test Bench</a>
    </header>

    <main>
      <div class="card">
        <div class="card-label">Cookie-Persisted Counter</div>
        <div class="count-display">""" + str(count) + """</div>
        <div class="count-label">total clicks</div>
        <form method="POST" action="/cookie-clicker">
          <button type="submit" class="click-btn">Click</button>
        </form>
        <div class="cookie-info">
          persisted via <code>click_count=""" + str(count) + """</code> cookie &mdash; survives page reloads
        </div>
      </div>
    </main>

    <footer>
      <div>WEBSERV COOKIE CLICKER &mdash; <code>42 Lausanne</code></div>
      <div>CGI &middot; COOKIES &middot; HTTP/1.1</div>
    </footer>
  </body>
</html>""")
