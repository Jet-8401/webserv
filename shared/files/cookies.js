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

async function waitForXAmountOfTime()
{
	const number = Math.floor(Math.random() * 1000);

	await sleep(number);
}

if (process.env.REQUEST_METHOD === "GET") {
	waitForXAmountOfTime();
	console.log("Hello World");
	process.exit(0);
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
		process.exit(0);
    });
}
