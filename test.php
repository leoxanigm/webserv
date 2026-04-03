#!/usr/bin/php
<?php
header("Content-Type: text/html");
echo "<html>\n";
echo "<head><title>PHP CGI Test</title></head>\n";
echo "<body>\n";
echo "<h1>PHP CGI Test Script</h1>\n";
echo "<p>Current time: " . date("Y-m-d H:i:s") . "</p>\n";
echo "<h2>Server Variables:</h2>\n";
echo "<pre>\n";
print_r($_SERVER);
echo "</pre>\n";
echo "<h2>Environment Variables:</h2>\n";
echo "<pre>\n";
print_r(getenv());
echo "</pre>\n";
echo "</body>\n";
echo "</html>\n";
?>
