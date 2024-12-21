<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>POST Data Handler</title>
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
        .data {
            text-align: left;
            margin: 20px;
            padding: 10px;
            background-color: #fff;
            border: 1px solid #eee;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>POST Data Received</h1>

        <?php
        // Get raw POST data
        $raw_data = file_get_contents('php://input');
        
        echo "<div class='data'>";
        echo "<h3>POST Variables:</h3>";
        if (!empty($_POST)) {
            foreach ($_POST as $key => $value) {
                echo "<p><strong>" . htmlspecialchars($key) . ":</strong> " . 
                     htmlspecialchars($value) . "</p>";
            }
        } else {
            echo "<p>No POST variables found.</p>";
        }

        echo "<h3>Raw POST Data:</h3>";
        if (!empty($raw_data)) {
            echo "<pre>" . htmlspecialchars($raw_data) . "</pre>";
        } else {
            echo "<p>No raw POST data found.</p>";
        }

        echo "<h3>Content Type:</h3>";
        echo "<p>" . htmlspecialchars($_SERVER['CONTENT_TYPE']) . "</p>";

        echo "<h3>Content Length:</h3>";
        echo "<p>" . htmlspecialchars($_SERVER['CONTENT_LENGTH']) . "</p>";
        echo "</div>";
        ?>
    </div>
</body>
</html>
