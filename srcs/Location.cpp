#include "../headers/Location.hpp"

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

    if (size.back() == 'M' || size.back() == 'm')
    {
        multiplier = 1024 * 1024;
        size.erase(size.size() - 1);
    }
    else if (size.back() == 'K' || size.back() == 'k')
    {
        multiplier = 1024;
        size.erase(size.size() - 1);
    }
    _client_max_body_size = atol(size.c_str()) * multiplier;
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
