/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_conf_parsing.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathboye <mathboye@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/03 13:12:26 by mathboye          #+#    #+#             */
/*   Updated: 2026/04/03 13:19:32 by mathboye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/parsing/ConfigFileParsing.hpp"
#include "../includes/tests.hpp"
#include "../includes/UI.hpp"
#include <iostream>
#include <list>
#include <string>
#include <algorithm>

static int g_conf_total = 0;
static int g_conf_pass  = 0;

static void printResult(const std::string &name, bool ok)
{
	++g_conf_total;
	if (ok)
	{
		++g_conf_pass;
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

static void expectEqualBool(const std::string &name, bool got, bool expected)
{
	printResult(name, got == expected);
	if (got != expected)
		std::cout << "  got:      [" << (got ? "true" : "false")
		          << "]\n  expected: [" << (expected ? "true" : "false") << "]" << std::endl;
}

static void expectEqualSize(const std::string &name, size_t got, size_t expected)
{
	printResult(name, got == expected);
	if (got != expected)
		std::cout << "  got:      [" << got << "]\n  expected: [" << expected << "]" << std::endl;
}

static void expectListContains(const std::string &name, const std::list<std::string> &lst, const std::string &val)
{
	bool found = std::find(lst.begin(), lst.end(), val) != lst.end();
	printResult(name, found);
	if (!found)
		std::cout << "  value [" << val << "] not found in list" << std::endl;
}

static void sectionHeader(const std::string &title)
{
	std::cout << "\n" << CYAN << "--- " << title << " ---" << RESET << std::endl;
}

void testConfParsing(void)
{
	g_conf_total = 0;
	g_conf_pass  = 0;
	std::cout << "\n===== ConfigFileParsing tests =====\n" << std::endl;

	// -------------------------------------------------------------------------
	// Empty / comment-only configs
	// -------------------------------------------------------------------------
	sectionHeader("Empty config");
	{
		ConfigFileParsing cfg("");
		expectTrue("empty config is valid", cfg.isValid());
		expectEqualSize("empty config has 0 servers", cfg.getServers().size(), 0);
	}

	sectionHeader("Comment-only config");
	{
		std::string raw = "# this is a comment\n# another comment\n";
		ConfigFileParsing cfg(raw);
		expectTrue("comment-only config is valid", cfg.isValid());
		expectEqualSize("comment-only has 0 servers", cfg.getServers().size(), 0);
	}

	// -------------------------------------------------------------------------
	// listen directive
	// -------------------------------------------------------------------------
	sectionHeader("listen port-only");
	{
		std::string raw =
			"server {\n"
			"    listen 8080;\n"
			"    server_name mysite.com;\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectTrue("port-only listen is valid", cfg.isValid());
		expectEqualSize("one server", cfg.getServers().size(), 1);
		const t_servers &srv = cfg.getServers().front();
		expectEqualStr("listen port", srv.listen_port, "8080");
		expectEqualStr("server_name", srv.server_name, "mysite.com");
		expectFalse("not default_server by default", srv.is_default_server);
	}

	sectionHeader("listen interface:port");
	{
		std::string raw =
			"server {\n"
			"    listen 0.0.0.0:9090;\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectTrue("interface:port listen is valid", cfg.isValid());
		const t_servers &srv = cfg.getServers().front();
		expectEqualStr("listen interface", srv.listen_interface, "0.0.0.0");
		expectEqualStr("listen port from interface:port", srv.listen_port, "9090");
	}

	sectionHeader("listen with default_server flag");
	{
		std::string raw =
			"server {\n"
			"    listen 80 default_server;\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectTrue("default_server config is valid", cfg.isValid());
		const t_servers &srv = cfg.getServers().front();
		expectTrue("is_default_server = true", srv.is_default_server);
		expectEqualStr("port with default_server", srv.listen_port, "80");
	}

	// -------------------------------------------------------------------------
	// error_page directive
	// -------------------------------------------------------------------------
	sectionHeader("error_page directives");
	{
		std::string raw =
			"server {\n"
			"    listen 8080;\n"
			"    error_page 404 /custom_404.html;\n"
			"    error_page 403 /custom_403.html;\n"
			"    error_page 500 /custom_500.html;\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectTrue("error_page config is valid", cfg.isValid());
		const t_servers &srv = cfg.getServers().front();
		expectEqualStr("error_page 404", srv.error_pages.NotFound, "/custom_404.html");
		expectEqualStr("error_page 403", srv.error_pages.Forbidden, "/custom_403.html");
		expectEqualStr("error_page 500", srv.error_pages.InternalServerError, "/custom_500.html");
	}

	// -------------------------------------------------------------------------
	// Multiple servers
	// -------------------------------------------------------------------------
	sectionHeader("Two servers");
	{
		std::string raw =
			"server {\n"
			"    listen 8080;\n"
			"    server_name site_a.com;\n"
			"}\n"
			"server {\n"
			"    listen 9090;\n"
			"    server_name site_b.com;\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectTrue("two servers config is valid", cfg.isValid());
		expectEqualSize("two servers count", cfg.getServers().size(), 2);
		std::list<t_servers>::const_iterator it = cfg.getServers().begin();
		expectEqualStr("first server name", it->server_name, "site_a.com");
		expectEqualStr("first server port", it->listen_port, "8080");
		++it;
		expectEqualStr("second server name", it->server_name, "site_b.com");
		expectEqualStr("second server port", it->listen_port, "9090");
	}

	// -------------------------------------------------------------------------
	// location directives
	// -------------------------------------------------------------------------
	sectionHeader("Location root and index");
	{
		std::string raw =
			"server {\n"
			"    listen 8080;\n"
			"    location / {\n"
			"        root ./www/html;\n"
			"        index index.html;\n"
			"    }\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectTrue("location root/index config is valid", cfg.isValid());
		const t_servers &srv = cfg.getServers().front();
		expectEqualSize("one location", srv.locations.size(), 1);
		const t_locations &loc = srv.locations.front();
		expectEqualStr("location path", loc.location, "/");
		expectEqualStr("location root", loc.root, "./www/html");
		expectEqualStr("location index", loc.index, "index.html");
	}

	sectionHeader("Location methods");
	{
		std::string raw =
			"server {\n"
			"    listen 8080;\n"
			"    location / {\n"
			"        methods GET POST DELETE;\n"
			"    }\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectTrue("location methods config is valid", cfg.isValid());
		const t_locations &loc = cfg.getServers().front().locations.front();
		expectEqualSize("3 methods", loc.allowed_methods.size(), 3);
		expectListContains("GET in methods", loc.allowed_methods, "GET");
		expectListContains("POST in methods", loc.allowed_methods, "POST");
		expectListContains("DELETE in methods", loc.allowed_methods, "DELETE");
	}

	sectionHeader("Autoindex on/off");
	{
		std::string raw =
			"server {\n"
			"    listen 8080;\n"
			"    location /files {\n"
			"        autoindex on;\n"
			"    }\n"
			"    location /hidden {\n"
			"        autoindex off;\n"
			"    }\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectTrue("autoindex config is valid", cfg.isValid());
		const t_servers &srv = cfg.getServers().front();
		expectEqualSize("two locations", srv.locations.size(), 2);
		std::list<t_locations>::const_iterator it = srv.locations.begin();
		expectEqualBool("autoindex on", it->autoindex, true);
		++it;
		expectEqualBool("autoindex off", it->autoindex, false);
	}

	sectionHeader("CGI PHP location");
	{
		std::string raw =
			"server {\n"
			"    listen 8080;\n"
			"    location /php {\n"
			"        cgi_php /usr/bin/php;\n"
			"    }\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectTrue("cgi_php config is valid", cfg.isValid());
		const t_locations &loc = cfg.getServers().front().locations.front();
		expectEqualBool("is_cgi true for php", loc.is_cgi, true);
		expectTrue("cgi_type is PHP", loc.cgi_type == PHP);
		expectEqualStr("cgi_location for php", loc.cgi_location, "/usr/bin/php");
	}

	sectionHeader("CGI Python location");
	{
		std::string raw =
			"server {\n"
			"    listen 8080;\n"
			"    location /py {\n"
			"        cgi_python /usr/bin/python3;\n"
			"    }\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectTrue("cgi_python config is valid", cfg.isValid());
		const t_locations &loc = cfg.getServers().front().locations.front();
		expectEqualBool("is_cgi true for python", loc.is_cgi, true);
		expectTrue("cgi_type is PYTHON", loc.cgi_type == PYTHON);
		expectEqualStr("cgi_location for python", loc.cgi_location, "/usr/bin/python3");
	}

	sectionHeader("Location return redirect");
	{
		std::string raw =
			"server {\n"
			"    listen 8080;\n"
			"    location /old {\n"
			"        return 301 /new;\n"
			"    }\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectTrue("return redirect config is valid", cfg.isValid());
		const t_locations &loc = cfg.getServers().front().locations.front();
		expectEqualUInt("return code 301", loc.return_code, 301);
		expectEqualStr("return location /new", loc.return_location, "/new");
	}

	sectionHeader("Two locations in one server");
	{
		std::string raw =
			"server {\n"
			"    listen 8080;\n"
			"    location / {\n"
			"        root /var/www/html;\n"
			"    }\n"
			"    location /api {\n"
			"        root /var/www/api;\n"
			"    }\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectTrue("two locations config is valid", cfg.isValid());
		const t_servers &srv = cfg.getServers().front();
		expectEqualSize("two locations in server", srv.locations.size(), 2);
		std::list<t_locations>::const_iterator it = srv.locations.begin();
		expectEqualStr("first location path", it->location, "/");
		++it;
		expectEqualStr("second location path", it->location, "/api");
	}

	// -------------------------------------------------------------------------
	// Comments
	// -------------------------------------------------------------------------
	sectionHeader("Comments are ignored");
	{
		std::string raw =
			"# top-level comment\n"
			"server { # inline comment\n"
			"    listen 8080; # port comment\n"
			"    server_name test.com; # name comment\n"
			"    # full-line comment inside block\n"
			"    location / {\n"
			"        root /var/www; # root comment\n"
			"    }\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectTrue("commented config is valid", cfg.isValid());
		const t_servers &srv = cfg.getServers().front();
		expectEqualStr("server_name past comments", srv.server_name, "test.com");
		expectEqualStr("port past comments", srv.listen_port, "8080");
		expectEqualStr("root past comments", srv.locations.front().root, "/var/www");
	}

	// -------------------------------------------------------------------------
	// Default values
	// -------------------------------------------------------------------------
	sectionHeader("Default values when directives absent");
	{
		std::string raw =
			"server {\n"
			"    listen 8080;\n"
			"    location / {\n"
			"    }\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectTrue("minimal server/location is valid", cfg.isValid());
		const t_servers &srv = cfg.getServers().front();
		expectFalse("default is_default_server = false", srv.is_default_server);
		const t_locations &loc = srv.locations.front();
		expectEqualStr("default root is ./", loc.root, "./");
		expectEqualStr("default index is index.html", loc.index, "index.html");
		expectEqualBool("default autoindex = false", loc.autoindex, false);
		expectEqualBool("default is_cgi = false", loc.is_cgi, false);
		expectTrue("default cgi_type = NONE", loc.cgi_type == NONE);
		expectEqualSize("default no allowed_methods", loc.allowed_methods.size(), 0);
	}

	// -------------------------------------------------------------------------
	// Invalid configs
	// -------------------------------------------------------------------------
	sectionHeader("Invalid: unclosed server block");
	{
		std::string raw =
			"server {\n"
			"    listen 8080;\n";
		ConfigFileParsing cfg(raw);
		expectFalse("unclosed server block is invalid", cfg.isValid());
	}

	sectionHeader("Invalid: unclosed location block");
	{
		std::string raw =
			"server {\n"
			"    listen 8080;\n"
			"    location / {\n"
			"        root /var/www;\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectFalse("unclosed location block is invalid", cfg.isValid());
	}

	sectionHeader("Invalid: location path without leading slash");
	{
		std::string raw =
			"server {\n"
			"    listen 8080;\n"
			"    location noSlash {\n"
			"        root /var/www;\n"
			"    }\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectFalse("location without leading / is invalid", cfg.isValid());
	}

	// -------------------------------------------------------------------------
	// Full realistic config
	// -------------------------------------------------------------------------
	sectionHeader("Full realistic config");
	{
		std::string raw =
			"server {\n"
			"    listen 8080;\n"
			"    listen 0.0.0.0:8080;\n"
			"    server_name webserv_a.com;\n"
			"    error_page 404 /404.html;\n"
			"\n"
			"    location / {\n"
			"        methods GET POST;\n"
			"        root ./www/html;\n"
			"        index index.html;\n"
			"    }\n"
			"\n"
			"    location /files {\n"
			"        methods POST;\n"
			"        root ./www/html/files;\n"
			"        autoindex on;\n"
			"    }\n"
			"\n"
			"    location /admin {\n"
			"        methods GET;\n"
			"        return 301 /home;\n"
			"    }\n"
			"}\n";
		ConfigFileParsing cfg(raw);
		expectTrue("full config is valid", cfg.isValid());
		expectEqualSize("full config: 1 server", cfg.getServers().size(), 1);
		const t_servers &srv = cfg.getServers().front();
		expectEqualStr("full config server_name", srv.server_name, "webserv_a.com");
		expectEqualStr("full config port", srv.listen_port, "8080");
		expectEqualStr("full config interface", srv.listen_interface, "0.0.0.0");
		expectEqualStr("full config error_page 404", srv.error_pages.NotFound, "/404.html");
		expectEqualSize("full config: 3 locations", srv.locations.size(), 3);

		std::list<t_locations>::const_iterator it = srv.locations.begin();
		expectEqualStr("loc[0] path", it->location, "/");
		expectEqualSize("loc[0] 2 methods", it->allowed_methods.size(), 2);
		expectListContains("loc[0] GET", it->allowed_methods, "GET");
		expectListContains("loc[0] POST", it->allowed_methods, "POST");

		++it;
		expectEqualStr("loc[1] path", it->location, "/files");
		expectEqualBool("loc[1] autoindex on", it->autoindex, true);

		++it;
		expectEqualStr("loc[2] path", it->location, "/admin");
		expectEqualUInt("loc[2] return code", it->return_code, 301);
		expectEqualStr("loc[2] return location", it->return_location, "/home");
	}

	// -------------------------------------------------------------------------
	// Summary
	// -------------------------------------------------------------------------
	std::cout << "\n" << BOLD_WHITE << "Config parsing: "
	          << g_conf_pass << "/" << g_conf_total << " tests passed." << RESET << std::endl;
}

