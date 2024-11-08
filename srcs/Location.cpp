#include "../headers/Location.hpp"
#include <iostream>

Location::Location(void):
    _methods(),
    _root(""),
    _error_pages(""),
    _autoindex(false),
    _client_max_body_size(0)
{
    std::cout << "Location constructor called" << std::endl;
}

void Location::setAutoindex(const std::string& value)
{
    _autoindex = (value == "on" || value == "ON" || value == "true");
}

void Location::setMethods(const std::string& value)
{
    _methods.insert(value);
}

void Location::setRoot(const std::string& value)
{
    _root = value;
}

void Location::setErrorPage(const std::string& value)
{
    _error_pages = value;
}

void Location::setClientMaxBodySize(const std::string& value)
{
    std::string size = value;
    long multiplier = 1;

    if (size[size.length() - 1] == 'M' || size[size.length() - 1] == 'm')
    {
        multiplier = 1024 * 1024;
        size.erase(size.size() - 1);
    }
    else if (size[size.length() - 1] == 'K' || size[size.length() - 1] == 'k')
    {
        multiplier = 1024;
        size.erase(size.size() - 1);
    }
    char* endptr;
    _client_max_body_size = std::strtol(size.c_str(), &endptr, 10) * multiplier;
}

// Getters
const std::set<std::string>& Location::getMethods(void) const
{
    return _methods;
}

const std::string& Location::getRoot(void) const
{
    return _root;
}

const std::string& Location::getErrorPages(void) const
{
    return _error_pages;
}

const bool& Location::getAutoIndex(void) const
{
    return _autoindex;
}

const long& Location::getClientMaxBodySize(void) const
{
    return _client_max_body_size;
}
