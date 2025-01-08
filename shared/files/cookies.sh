#!/bin/bash
VISIT_COUNT=""
IFS=';' read -r -a cookies <<< "$HTTP_COOKIE"
echo "Content-Type: text/html"
for cookie in "${cookies[@]}"; do
  cookie=$(echo "$cookie" | sed 's/^ *//;s/ *$//')
  IFS='=' read -r -a cookie_parts <<< "$cookie"
  if [ "${cookie_parts[0]}" = "visit" ]; then
    VISIT_COUNT="${cookie_parts[1]}"
  fi
done
if [ -z "$VISIT_COUNT" ]; then
  VISIT_COUNT=1
else
  VISIT_COUNT=$((VISIT_COUNT + 1))
fi
EXPIRATION_DATE=$(date -u -d "+5 minutes" +"%a, %d-%b-%Y %H:%M:%S GMT")
echo -n "Set-Cookie: visit=$VISIT_COUNT; Expires=$EXPIRATION_DATE; Path=/"
echo -en "\r\n\r\n"
echo "<!DOCTYPE html>"
echo "<html lang=\"en\">"
echo "<head>"
echo "    <meta charset=\"UTF-8\">"
echo "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
echo "    <title>Visit Count</title>"
echo "    <style>"
echo "        body {"
echo "            font-family: Arial, sans-serif;"
echo "            margin: 0;"
echo "            padding: 0;"
echo "            display: flex;"
echo "            align-items: center;"
echo "            justify-content: center;"
echo "            height: 100vh;"
echo "            background: linear-gradient(to bottom right, #F3F4F7, #D3DCE6);"
echo "            color: #333;"
echo "        }"
echo "        .container {"
echo "            background: #fff;"
echo "            padding: 40px;"
echo "            border-radius: 12px;"
echo "            box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);"
echo "            text-align: center;"
echo "            max-width: 400px;"
echo "        }"
echo "        h1 {"
echo "            font-size: 2em;"
echo "            color: #0078D7;"
echo "            margin: 0;"
echo "        }"
echo "        p {"
echo "            margin-top: 10px;"
echo "            font-size: 1.2em;"
echo "            color: #555;"
echo "        }"
echo "    </style>"
echo "</head>"
echo "<body>"
echo "    <div class=\"container\">"
if [ "$VISIT_COUNT" -eq "1" ]; then
  echo "<h1>Welcome</h1>"
else
  echo "<h1>Welcome Back!</h1>"
fi
echo "        <p>You have visited this page: <strong>$VISIT_COUNT times</strong></p>"
echo "    </div>"
echo "</body>"
