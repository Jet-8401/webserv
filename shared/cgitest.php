<?php
// Debug to error log
error_log("Script started - REQUEST_METHOD: " . $_SERVER['REQUEST_METHOD']);

// Required headers for CGI
header("Content-type: text/html");

// Handle POST data
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Try to get POST data from standard input
    $input = fopen("php://input", "r");
    $raw_data = '';
    while (!feof($input)) {
        $raw_data .= fread($input, 1024);
    }
    fclose($input);
    error_log("Raw POST data: " . $raw_data);

    // Parse POST data
    parse_str($raw_data, $post_data);
    error_log("Parsed POST data: " . print_r($post_data, true));

    if (!empty($post_data['cookie_name']) && !empty($post_data['cookie_value'])) {
        // Send cookie header
        echo "Set-Cookie: " . urlencode($post_data['cookie_name']) . "=" . 
             urlencode($post_data['cookie_value']) . "; Path=/\r\n";
    }
}

// End of headers
echo "\r\n";
?>
<!DOCTYPE html>
<html>
<head>
    <title>CGI Test</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 40px;
        }
        .container {
            border: 1px solid #ddd;
            padding: 20px;
            border-radius: 5px;
        }
        .section {
            margin-bottom: 20px;
            padding: 10px;
            background-color: #f5f5f5;
        }
        .cookie-form {
            margin-top: 20px;
            padding: 10px;
            background-color: #e9e9e9;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>CGI Environment Test</h1>

        <!-- Display CGI environment variables -->
        <div class="section">
            <h2>CGI Environment Variables:</h2>
            <?php
            $important_vars = array(
                'GATEWAY_INTERFACE',
                'SERVER_PROTOCOL',
                'REQUEST_METHOD',
                'SCRIPT_NAME',
                'QUERY_STRING',
                'PATH_INFO',
                'HTTP_COOKIE',
                'CONTENT_TYPE',
                'CONTENT_LENGTH',
                'SERVER_NAME',
                'SERVER_PORT'
            );

            foreach ($important_vars as $var) {
                echo "<strong>$var:</strong> " . 
                     (isset($_SERVER[$var]) ? htmlspecialchars($_SERVER[$var]) : "Not set") . 
                     "<br/>\n";
            }
            ?>
        </div>

        <!-- Display POST data -->
        <div class="section">
            <h2>POST Data:</h2>
            <?php
            if ($_SERVER['REQUEST_METHOD'] === 'POST') {
                echo "<pre>";
                echo "Raw POST data: " . htmlspecialchars($raw_data) . "\n\n";
                echo "Parsed POST data:\n";
                print_r($post_data);
                echo "</pre>";
            } else {
                echo "No POST data (not a POST request)";
            }
            ?>
        </div>

        <!-- Display current cookies -->
        <div class="section">
            <h2>Current Cookies:</h2>
            <?php
            if (empty($_COOKIE)) {
                echo "No cookies set";
            } else {
                foreach ($_COOKIE as $name => $value) {
                    echo "<strong>" . htmlspecialchars($name) . ":</strong> " . 
                         htmlspecialchars($value) . "<br/>\n";
                }
            }
            ?>
        </div>

        <!-- Cookie setting form -->
        <div class="cookie-form">
            <h2>Set a Cookie:</h2>
            <form method="POST" action="">
                <p>
                    <label>Cookie Name: <input type="text" name="cookie_name" required></label>
                </p>
                <p>
                    <label>Cookie Value: <input type="text" name="cookie_value" required></label>
                </p>
                <p>
                    <input type="submit" value="Set Cookie">
                </p>
            </form>
        </div>

        <!-- Display headers info -->
        <div class="section">
            <h2>Headers Information:</h2>
            <?php
            if ($_SERVER['REQUEST_METHOD'] === 'POST' && 
                !empty($post_data['cookie_name']) && 
                !empty($post_data['cookie_value'])) {
                echo "<pre>";
                echo "Set-Cookie header sent: \n";
                echo htmlspecialchars($cookie_header);
                echo "</pre>";
            }
            ?>
        </div>
    </div>
</body>
</html>
