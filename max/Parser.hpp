#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>
# include <fstream>
#include <stdlib.h>
# include "Config.hpp"

class Parser
{
    private:
        std::string _filename;
        std::ifstream _file;
        int _line_number;  // for error reporting

        void parseHttp(HttpConfig& config);
        void parseServer(ServerConfig& server);
        void parseLocation(LocationConfig& location);

        // Helper methods
        void skipWhitespace();
        std::string getNextToken();
        bool isOpenBrace(const std::string& token);
        bool isCloseBrace(const std::string& token);

    public:
        Parser(const std::string& filename);
        ~Parser();

        HttpConfig parseConfig();
};

#endif
