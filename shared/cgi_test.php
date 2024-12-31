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

        <!-- Display all cookies -->
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

        <!-- Form to set a cookie -->
        <div class="cookie-form">
            <h2>Set a Cookie:</h2>
            <?php
            if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['cookie_name']) && isset($_POST['cookie_value'])) {
                $cookie_name = $_POST['cookie_name'];
                $cookie_value = $_POST['cookie_value'];
                setcookie($cookie_name, $cookie_value, time() + 3600);
                echo "<p>Cookie set! Refresh to see it in the list above.</p>";
            }
            ?>
            <form method="POST">
                <p>
                    <label>Cookie Name: <input type="text" name="cookie_name"></label>
                </p>
                <p>
                    <label>Cookie Value: <input type="text" name="cookie_value"></label>
                </p>
                <p>
                    <input type="submit" value="Set Cookie">
                </p>
            </form>
        </div>
    </div>
</body>
</html>
