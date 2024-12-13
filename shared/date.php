<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Server Time and Location</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin-top: 50px;
        }
        .container {
            display: inline-block;
            padding: 20px;
            border: 1px solid #ddd;
            border-radius: 5px;
            background-color: #f9f9f9;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Server Time and Location</h1>

        <?php
        // Get current server time
        date_default_timezone_set("UTC"); // Set timezone to UTC
        $current_time = date("Y-m-d H:i:s");
        ?>

        <p><strong>Current Server Time:</strong> <?php echo htmlspecialchars(
        	$current_time
        ); ?></p>
    </div>
</body>
</html>
