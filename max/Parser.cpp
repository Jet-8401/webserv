#include "Parser.hpp"
#include <vector>

Parser::Parser(const std::string& filename)
    : _filename(filename), _line_number(1)
{
    _file.open(filename.c_str());
}

Parser::~Parser() {
    if (_file.is_open())
        _file.close();
}

std::string Parser::getNextToken() {
    std::string token;
    char c;

    // Skip whitespace
    while (_file.get(c)) {
        if (c == '\n')
            _line_number++;
        if (!isspace(c)) {
            _file.putback(c);
            break;
        }
    }

    // Read token
    while (_file.get(c)) {
        if (c == '\n') {
            _line_number++;
            break;
        }
        if (c == '{' || c == '}' || c == ';') {
            if (token.empty())
                token = c;
            else
                _file.putback(c);
            break;
        }
        if (isspace(c))
            break;
        token += c;
    }

    return token;
}

HttpConfig Parser::parseConfig() {
    HttpConfig config;
    std::string token = getNextToken();  // Should be "http"
    token = getNextToken();              // Should be "{"

    parseHttp(config);
    return config;
}

void Parser::parseHttp(HttpConfig& config) {
    std::string token;

    while ((token = getNextToken()) != "}") {
        if (token == "server") {
            ServerConfig server;
            token = getNextToken();  // Should be "{"
            parseServer(server);
            config.servers.push_back(server);
        }
        else if (token == "client_max_body_size") {
            token = getNextToken();  // Get the value
            config.client_max_body_size.set(atoi(token.c_str()));
            token = getNextToken();  // Should be ";"
        }
    }
}

void Parser::parseServer(ServerConfig& server) {
    std::string token;

    while ((token = getNextToken()) != "}") {
        if (token == "location") {
            LocationConfig location;
            token = getNextToken();  // Get location path
            location.path.set(token);
            token = getNextToken();  // Should be "{"
            parseLocation(location);
            server.locations[location.path.getValue()] = location;
        }
        else if (token == "listen") {
            token = getNextToken();
            server.port.set(atoi(token.c_str()));
            token = getNextToken();  // Should be ";"
        }
        else if (token == "root") {
            token = getNextToken();
            server.root.set(token);
            token = getNextToken();  // Should be ";"
        }
        else if (token == "server_name") {
            std::vector<std::string> names;
            while ((token = getNextToken()) != ";") {
                names.push_back(token);
            }
            server.server_names.set(names);
        }
        else if (token == "error_page") {
            if (!server.error_pages.isSet()) {
                server.error_pages.set(std::map<int, std::string>());
            }
            std::vector<int> error_codes;
            while ((token = getNextToken()) != ";") {
                if (token.find('/') != std::string::npos) {
                    // This is the path, assign it to all collected error codes
                    for (size_t i = 0; i < error_codes.size(); ++i) {
                        server.error_pages.getValue()[error_codes[i]] = token;
                    }
                    break;
                }
                // Must be an error code
                error_codes.push_back(atoi(token.c_str()));
            }
        }
        // Add other server directives here
    }
}

void Parser::parseLocation(LocationConfig& location) {
    std::string token;

    while ((token = getNextToken()) != "}") {
        if (token == "autoindex") {
            token = getNextToken();
            location.autoindex.set(token == "on");
            token = getNextToken();  // Should be ";"
        }
        else if (token == "allowed_methods") {
            std::vector<std::string> methods;
            while ((token = getNextToken()) != ";") {
                methods.push_back(token);
            }
            location.allowed_methods.set(methods);
        }
        else if (token == "root") {
            token = getNextToken();
            location.root.set(token);
            token = getNextToken();  // Should be ";"
        }
        // Add other location directives here
    }
}
