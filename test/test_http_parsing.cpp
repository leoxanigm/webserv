#include "../includes/parsing/HTTPRequestParser.hpp"
#include "../includes/UI.hpp"
#include <iostream>
#include <map>
#include <string>

static int g_total = 0;
static int g_pass = 0;

static void printResult(const std::string &name, bool ok)
{
	++g_total;
	if (ok)
	{
		++g_pass;
		std::cout << GREEN << "[PASS] " << RESET << name << std::endl;
	}
	else
		std::cout << RED << "[FAIL] " << RESET << name << std::endl;
}

static void expectTrue(const std::string &name, bool value)
{
	printResult(name, value == true);
}

static void expectFalse(const std::string &name, bool value)
{
	printResult(name, value == false);
}

static void expectEqualStr(const std::string &name, const std::string &got, const std::string &expected)
{
	printResult(name, got == expected);
	if (got != expected)
		std::cout << "  got:      [" << got << "]\n  expected: [" << expected << "]" << std::endl;
}

static void expectEqualUInt(const std::string &name, unsigned int got, unsigned int expected)
{
	printResult(name, got == expected);
	if (got != expected)
		std::cout << "  got:      [" << got << "]\n  expected: [" << expected << "]" << std::endl;
}

static void expectEqualMethod(const std::string &name, t_http_methods got, t_http_methods expected)
{
	printResult(name, got == expected);
	if (got != expected)
		std::cout << "  got:      [" << got << "]\n  expected: [" << expected << "]" << std::endl;
}

static void expectMapContainsStrStr(
	const std::string &name,
	const std::map<std::string, std::string> &m,
	const std::string &key,
	const std::string &expectedValue)
{
	std::map<std::string, std::string>::const_iterator it = m.find(key);
	bool ok = (it != m.end() && it->second == expectedValue);
	printResult(name, ok);
	if (!ok)
	{
		std::cout << "  missing or mismatched key [" << key << "]" << std::endl;
		if (it != m.end())
			std::cout << "  got value: [" << it->second << "] expected: [" << expectedValue << "]" << std::endl;
	}
}

static void expectMapContainsStrFloat(
	const std::string &name,
	const std::map<std::string, float> &m,
	const std::string &key,
	float expectedValue)
{
	std::map<std::string, float>::const_iterator it = m.find(key);
	bool ok = (it != m.end() && it->second == expectedValue);
	printResult(name, ok);
	if (!ok)
	{
		std::cout << "  missing or mismatched key [" << key << "]" << std::endl;
		if (it != m.end())
			std::cout << "  got value: [" << it->second << "] expected: [" << expectedValue << "]" << std::endl;
	}
}

void testHttpParsing(void)
{
	std::cout << "\n===== HTTPRequest parser tests =====\n" << std::endl;

	// ------------------------------------------------------------
	// VALID: minimal GET
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET / HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectTrue("valid minimal GET", req.isValid());
		expectEqualMethod("GET method parsed", req.getMethod(), GET);
		expectEqualStr("GET target parsed", req.getRequestTarget(), "/");
		expectEqualStr("GET version parsed", req.getHTTPVersion(), "HTTP/1.1");
		expectEqualStr("GET host parsed", req.getHost(), "example.com");
		expectEqualStr("GET body empty", req.getBody(), "");
		expectEqualUInt("GET content length default", req.getContentLength(), 0);
	}

	// ------------------------------------------------------------
	// VALID: GET with common headers
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET /index.html HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"User-Agent: Mozilla/5.0\r\n"
			"Accept: text/html,application/xhtml+xml,*/*\r\n"
			"Accept-Language: en-US;q=1.0,en;q=0.8,fr;q=0.6\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectTrue("valid GET with headers", req.isValid());
		expectEqualMethod("GET with headers method", req.getMethod(), GET);
		expectEqualStr("GET with headers target", req.getRequestTarget(), "/index.html");
		expectEqualStr("GET with headers version", req.getHTTPVersion(), "HTTP/1.1");
		expectEqualStr("GET with headers host", req.getHost(), "localhost:8080");
		expectEqualStr("GET with headers user-agent", req.getUserAgent(), "Mozilla/5.0");

		// These assertions assume your parser stores tokens as-is or near-as-is.
		// Adjust keys if your implementation normalizes them differently.
		expectMapContainsStrStr("Accept contains text/html", req.getAccept(), "text/html", "");
		expectMapContainsStrStr("Accept contains application/xhtml+xml", req.getAccept(), "application/xhtml+xml", "");
		expectMapContainsStrStr("Accept contains */*", req.getAccept(), "*/*", "");

		expectMapContainsStrFloat("Accept-Language contains en-US", req.getLanguages(), "en-US", 1.0f);
		expectMapContainsStrFloat("Accept-Language contains en", req.getLanguages(), "en", 0.8f);
		expectMapContainsStrFloat("Accept-Language contains fr", req.getLanguages(), "fr", 0.6f);
	}

	// ------------------------------------------------------------
	// VALID: POST with body and content headers
	// ------------------------------------------------------------
	{
		std::string body = "name=zola&project=webserv";
		std::string raw =
			"POST /submit HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Content-Length: 25\r\n"
			"Content-Type: application/x-www-form-urlencoded; charset=UTF-8\r\n"
			"\r\n"
			"name=zola&project=webserv";

		HTTPRequest req(raw);
		expectTrue("valid POST with body", req.isValid());
		expectEqualMethod("POST method parsed", req.getMethod(), POST);
		expectEqualStr("POST target parsed", req.getRequestTarget(), "/submit");
		expectEqualStr("POST version parsed", req.getHTTPVersion(), "HTTP/1.1");
		expectEqualStr("POST host parsed", req.getHost(), "example.com");
		expectEqualUInt("POST content-length parsed", req.getContentLength(), 25);
		expectEqualStr("POST body parsed", req.getBody(), body);

		expectMapContainsStrStr("Content-Type contains media type",
			req.getContentType(), "application/x-www-form-urlencoded", "");
		expectMapContainsStrStr("Content-Type contains charset",
			req.getContentType(), "charset", "UTF-8");
	}

	// ------------------------------------------------------------
	// VALID: POST with empty body and Content-Length 0
	// ------------------------------------------------------------
	{
		std::string raw =
			"POST /empty HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Content-Length: 0\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectTrue("valid POST with empty body", req.isValid());
		expectEqualMethod("POST empty body method", req.getMethod(), POST);
		expectEqualUInt("POST empty body content-length", req.getContentLength(), 0);
		expectEqualStr("POST empty body parsed", req.getBody(), "");
	}

	// ------------------------------------------------------------
	// VALID: DELETE
	// ------------------------------------------------------------
	{
		std::string raw =
			"DELETE /resource/42 HTTP/1.1\r\n"
			"Host: api.example.com\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectTrue("valid DELETE", req.isValid());
		expectEqualMethod("DELETE method parsed", req.getMethod(), DELETE);
		expectEqualStr("DELETE target parsed", req.getRequestTarget(), "/resource/42");
		expectEqualStr("DELETE host parsed", req.getHost(), "api.example.com");
	}

	// ------------------------------------------------------------
	// INVALID: unsupported method
	// ------------------------------------------------------------
	{
		std::string raw =
			"PUT / HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid unsupported method PUT", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: lowercase method
	// ------------------------------------------------------------
	{
		std::string raw =
			"get / HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid lowercase method", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: missing method
	// ------------------------------------------------------------
	{
		std::string raw =
			"/ HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid missing method in request line", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: missing target
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid missing target in request line", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: missing version
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET /\r\n"
			"Host: example.com\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid missing version in request line", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: extra token in request line
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET / HTTP/1.1 EXTRA\r\n"
			"Host: example.com\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid extra token in request line", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: malformed HTTP version
	// ------------------------------------------------------------
	{
		std::string raw1 =
			"GET / HTTP/1.0\r\n"
			"Host: example.com\r\n"
			"\r\n";

		HTTPRequest req1(raw1);
		// If your implementation accepts HTTP/1.0, flip this expectation.
		expectFalse("invalid unsupported HTTP/1.0 if parser only supports 1.1", req1.isValid());

		std::string raw2 =
			"GET / HTTP/2.0\r\n"
			"Host: example.com\r\n"
			"\r\n";

		HTTPRequest req2(raw2);
		expectFalse("invalid unsupported HTTP/2.0", req2.isValid());

		std::string raw3 =
			"GET / HTP/1.1\r\n"
			"Host: example.com\r\n"
			"\r\n";

		HTTPRequest req3(raw3);
		expectFalse("invalid malformed version token", req3.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: missing Host
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET / HTTP/1.1\r\n"
			"User-Agent: test\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid missing Host header", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: malformed header line without colon
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET / HTTP/1.1\r\n"
			"Host example.com\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid header without colon", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: empty header name
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET / HTTP/1.1\r\n"
			": example.com\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid empty header key", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: duplicate Host header
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET / HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Host: duplicate.com\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid duplicate Host header", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: Content-Length non-numeric
	// ------------------------------------------------------------
	{
		std::string raw =
			"POST /upload HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Content-Length: abc\r\n"
			"\r\n"
			"body";

		HTTPRequest req(raw);
		expectFalse("invalid Content-Length non-numeric", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: Content-Length negative
	// ------------------------------------------------------------
	{
		std::string raw =
			"POST /upload HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Content-Length: -1\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid Content-Length negative", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: Content-Length overflow
	// ------------------------------------------------------------
	{
		std::string raw =
			"POST /upload HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Content-Length: 999999999999999999999999\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid Content-Length overflow", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: body shorter than Content-Length
	// ------------------------------------------------------------
	{
		std::string raw =
			"POST /submit HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Content-Length: 10\r\n"
			"\r\n"
			"abc";

		HTTPRequest req(raw);
		expectFalse("invalid body shorter than Content-Length", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: body longer than Content-Length
	// ------------------------------------------------------------
	{
		std::string raw =
			"POST /submit HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Content-Length: 3\r\n"
			"\r\n"
			"abcdef";

		HTTPRequest req(raw);
		expectFalse("invalid body longer than Content-Length", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: POST body but no Content-Length
	// ------------------------------------------------------------
	{
		std::string raw =
			"POST /submit HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"\r\n"
			"abc";

		HTTPRequest req(raw);
		expectFalse("invalid POST body without Content-Length", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: Content-Type without POST body context
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET / HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Content-Type: text/plain\r\n"
			"\r\n";

		HTTPRequest req(raw);
		// Some parsers allow this, some reject it.
		// Keep as invalid if your design forbids entity headers on GET.
		expectFalse("invalid GET with Content-Type if parser forbids it", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: malformed Accept-Language q value
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET / HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Accept-Language: en-US;q=abc,en;q=0.8\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid Accept-Language malformed q", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: malformed Accept header
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET / HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Accept: text/html;=broken\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid malformed Accept header", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: malformed Content-Type header
	// ------------------------------------------------------------
	{
		std::string raw =
			"POST /submit HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Content-Length: 4\r\n"
			"Content-Type: ; charset=UTF-8\r\n"
			"\r\n"
			"test";

		HTTPRequest req(raw);
		expectFalse("invalid malformed Content-Type header", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: missing CRLF CRLF separator
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET / HTTP/1.1\r\n"
			"Host: example.com\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid missing header/body separator", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: completely empty request
	// ------------------------------------------------------------
	{
		std::string raw = "";
		HTTPRequest req(raw);
		expectFalse("invalid empty request", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: only CRLF
	// ------------------------------------------------------------
	{
		std::string raw = "\r\n";
		HTTPRequest req(raw);
		expectFalse("invalid blank request", req.isValid());
	}

	// ------------------------------------------------------------
	// VALID/INVALID depending on your parser: absolute URI target
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET http://example.com/path HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"\r\n";

		HTTPRequest req(raw);
		// If your parser only accepts origin-form targets, this should be invalid.
		expectFalse("invalid absolute-form target if parser only accepts origin-form", req.isValid());
	}

	// ------------------------------------------------------------
	// VALID/INVALID depending on your parser: query string
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET /search?q=webserv&lang=en HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectTrue("valid GET with query string", req.isValid());
		expectEqualStr("query string target preserved", req.getRequestTarget(), "/search?q=webserv&lang=en");
	}

	// ------------------------------------------------------------
	// VALID: headers with surrounding spaces after colon
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET / HTTP/1.1\r\n"
			"Host:    example.com   \r\n"
			"User-Agent:   test-agent   \r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectTrue("valid header values with extra spaces", req.isValid());
		// If your parser trims values:
		expectEqualStr("trimmed Host value", req.getHost(), "example.com");
		expectEqualStr("trimmed User-Agent value", req.getUserAgent(), "test-agent");
	}

	// ------------------------------------------------------------
	// VALID: chunked request
	// ------------------------------------------------------------
	{
		std::string raw =
			"POST /upload HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Transfer-Encoding: chunked\r\n"
			"\r\n"
			"4\r\n"
			"Wiki\r\n"
			"5\r\n"
			"pedia\r\n"
			"0\r\n"
			"\r\n";

		HTTPRequest req(raw);
		expectTrue("valid chunked request", req.isValid());
	}

	// ------------------------------------------------------------
	// INVALID: chunked request
	// ------------------------------------------------------------
	{
		std::string raw =
			"POST /upload HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Content-Length: 9\r\n"
			"Transfer-Encoding: chunked\r\n"
			"\r\n"
			"4\r\n"
			"Wiki\r\n";

		HTTPRequest req(raw);
		expectFalse("invalid request with both Content-Length and Transfer-Encoding", req.isValid());
	}

	// ------------------------------------------------------------
	// VALID: POST with file
	// ------------------------------------------------------------
	{
		std::string body =
			"------WebKitFormBoundaryABC123\r\n"
			"Content-Disposition: form-data; name=\"file\"; filename=\"hello.txt\"\r\n"
			"Content-Type: text/plain\r\n"
			"\r\n"
			"hello world\n"
			"------WebKitFormBoundaryABC123--\r\n";

		std::ostringstream oss;
		oss << body.size();

		std::string raw =
			"POST /upload HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Content-Length: " + oss.str() + "\r\n"
			"Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryABC123\r\n"
			"\r\n" +
			body;

		HTTPRequest req(raw);
		expectTrue("valid multipart/form-data POST", req.isValid());
		expectEqualMethod("multipart POST method", req.getMethod(), POST);
		expectEqualStr("multipart POST target", req.getRequestTarget(), "/upload");
		expectEqualStr("multipart POST host", req.getHost(), "example.com");
		expectEqualUInt("multipart POST content length", req.getContentLength(), body.size());
		expectEqualStr("multipart POST body preserved", req.getBody(), body);

		expectMapContainsStrStr("multipart Content-Type media type",
			req.getContentType(), "multipart/form-data", "");
		expectMapContainsStrStr("multipart Content-Type boundary",
			req.getContentType(), "boundary", "----WebKitFormBoundaryABC123");
	}

	// ------------------------------------------------------------
	// INVALID: multipart missing boundary
	// ------------------------------------------------------------

	{
		std::string body = "some body";
		std::ostringstream oss;
		oss << body.size();

		std::string raw =
			"POST /upload HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Content-Length: " + oss.str() + "\r\n"
			"Content-Type: multipart/form-data\r\n"
			"\r\n" +
			body;

		HTTPRequest req(raw);
		expectFalse("invalid multipart/form-data without boundary", req.isValid());
	}

	// ------------------------------------------------------------
	// COPY CONSTRUCTOR
	// ------------------------------------------------------------
	{
		std::string raw =
			"GET /copy HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"User-Agent: copier\r\n"
			"\r\n";

		HTTPRequest req(raw);
		HTTPRequest copy(req);

		expectTrue("copy constructor preserves validity", copy.isValid());
		expectEqualMethod("copy constructor preserves method", copy.getMethod(), GET);
		expectEqualStr("copy constructor preserves target", copy.getRequestTarget(), "/copy");
		expectEqualStr("copy constructor preserves version", copy.getHTTPVersion(), "HTTP/1.1");
		expectEqualStr("copy constructor preserves host", copy.getHost(), "example.com");
		expectEqualStr("copy constructor preserves UA", copy.getUserAgent(), "copier");
	}

	// ------------------------------------------------------------
	// ASSIGNMENT OPERATOR
	// ------------------------------------------------------------
	{
		std::string raw =
			"DELETE /assigned HTTP/1.1\r\n"
			"Host: example.org\r\n"
			"\r\n";

		HTTPRequest req(raw);
		HTTPRequest assigned;
		assigned = req;

		expectTrue("assignment preserves validity", assigned.isValid());
		expectEqualMethod("assignment preserves method", assigned.getMethod(), DELETE);
		expectEqualStr("assignment preserves target", assigned.getRequestTarget(), "/assigned");
		expectEqualStr("assignment preserves host", assigned.getHost(), "example.org");
		expectEqualStr("assignment preserves version", assigned.getHTTPVersion(), "HTTP/1.1");
	}

	std::cout << "\n===== HTTPRequest parser tests: "
			  << g_pass << "/" << g_total << " passed =====\n" << std::endl;
}