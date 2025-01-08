<?php
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    echo "Content-Type: text/html\r\n";
    echo "Set-Cookie: " . $_POST['cookie_name'] . "=" . $_POST['cookie_value'] . "; Path=/\r\n";
    echo "\r\n";
} else {
    echo "Content-Type: text/html\r\n";
    echo "\r\n";
}
?>
<html>
<head><title>Cookie Test</title></head>
<body>
    <form method="POST">
        <p><input type="text" name="cookie_name" placeholder="Cookie Name"></p>
        <p><input type="text" name="cookie_value" placeholder="Cookie Value"></p>
        <p><input type="submit" value="Set Cookie"></p>
    </form>
</body>
</html>
