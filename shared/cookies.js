// Function to parse URL-encoded data
function parseFormData(formData) {
    const pairs = formData.split('&');
    const result = {};

    pairs.forEach(pair => {
        const [key, value] = pair.split('=');
        result[decodeURIComponent(key)] = decodeURIComponent(value);
    });

    return result;
}

if (process.env.REQUEST_METHOD === "GET") {
	console.log("Hello World");
	while (1) { }
}

// Handle POST method
if (process.env.REQUEST_METHOD === "POST") {
    let postData = '';
    process.stdin.on('data', chunk => {
        postData += chunk.toString();

        const formData = parseFormData(postData);

        console.log("Content-Type: text/html\r\n\r\n");
        console.log("<html><body>");
        console.log("<h1>POST Data:</h1>");
        console.log("<pre>");
        console.log(JSON.stringify(formData, null, 2));
        console.log("</pre>");
        console.log("</body></html>");
    });
}
