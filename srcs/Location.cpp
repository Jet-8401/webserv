#include "../headers/Location.hpp"
#include <iostream>
#include <sstream>
#include <vector>

Location::Location(void):
    _methods(),
    _root(),
    _error_pages(),
    _autoindex(false),
    _client_max_body_size(0)
{
    std::cout << "Location constructor called" << std::endl;
}

void Location::setAutoindex(const std::string& value)
{
	std::cout <<"autoindex value :"<< value << std::endl;
    _autoindex = (value == "on" || value == "ON" || value == "true");
}

void Location::setMethods(const std::string& value)
{
	std::istringstream iss(value);
	std::string method;
	while (iss >> method)
	{
		_methods.insert(method);
	}
}

void Location::setRoot(const std::string& value)
{
    _root = value;
}

void Location::setCgis(const std::string& value)
{
	std::istringstream iss(value);
	std::string ext;
	std::string path;
	if (!(iss >> ext >> path) || !iss.eof())
        throw std::runtime_error("Invalid CGI format");
    if (ext[0] != '.')
        throw std::runtime_error("CGI extension must be valid");
    _cgis[ext] = path;
}

void Location::setErrorPage(const std::string& value)
{
	std::istringstream iss(value);
	std::vector<std::string> words;
	std::string word;
	while (iss >> word)
	{
		words.push_back(word);
	}
	if (words.size() < 2)
    {
		return; // or throw an exception if you prefer
	}
	std::string* valuePtr = new std::string(words[words.size() - 1]);
	for (size_t i = 0; i < words.size() - 1; ++i)
	{
        _error_pages[words[i]] = valuePtr;
    }
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

const std::map<std::string, std::string*>& Location::getErrorPages(void) const
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
