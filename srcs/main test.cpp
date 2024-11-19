#include "../headers/HttpResponse.hpp"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

// Helper function to print response content
void printResponseContent(const std::string& response) {
    std::cout << "Response length: " << response.length() << " bytes\n";
    std::cout << "Content:\n" << response << "\n---END---\n";
}

void testResponse(const char* test_name) {
    std::cout << "\n=== " << test_name << " ===\n";

    int sockets[2];
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sockets) == -1) {
        std::cerr << "Failed to create socket pair" << std::endl;
        return;
    }

    HttpResponse response;

    // Test specific response setup based on test name
    if (strcmp(test_name, "Basic Response") == 0) {
        response.setHeader("Content-Type", "text/plain");
        response.setBody("Hello, World!");
    }
    else if (strcmp(test_name, "404 Error") == 0) {
        response.setHeader("Content-Type", "text/html");
        response.setBody("<html><body><h1>404 Not Found</h1></body></html>");
    }
    // ... add other test cases

    // Send response
    if (response.send(sockets[1]) == -1) {
        std::cerr << "Failed to send response" << std::endl;
        close(sockets[0]);
        close(sockets[1]);
        return;
    }

    // Read response with timeout
    char buffer[4096];
    std::string received;
    ssize_t bytes;

    // Single read attempt (non-blocking)
    bytes = read(sockets[0], buffer, sizeof(buffer));
    if (bytes > 0) {
        received.append(buffer, bytes);
    }

    printResponseContent(received);

    close(sockets[0]);
    close(sockets[1]);
}

int main() {
    std::cout << "Starting HttpResponse Tests\n";

    testResponse("Basic Response");
    testResponse("404 Error");
    // Add more test cases as needed

    std::cout << "\nAll tests completed.\n";
    return 0;
}
