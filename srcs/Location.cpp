#include "../headers/Location.hpp"
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <string.h>

Location::Location(void):
	_autoindex(false),
	_client_max_body_size(32768)
{
	this->_return.first = 0;
	this->_methods.insert("GET");
}

Location::Location(const Location& src):
	_methods(src._methods),
	_root(src._root),
	_autoindex(src._autoindex),
	_client_max_body_size(src._client_max_body_size),
	_cgis(src._cgis),
	_index(src._index),
	_return(src._return),
	_alias(src._alias)
{
	// do deep copy
	std::map<std::string*, std::string*>		ptr_copy;
	std::map<int, std::string*>::const_iterator	it;

	for (it = src._error_pages.begin(); it != src._error_pages.end(); it++) {
		if (ptr_copy.find(it->second) != ptr_copy.end())
			continue ;
		ptr_copy[it->second] = new std::string(*it->second);
	}

	for (it = src._error_pages.begin(); it != src._error_pages.end(); it++) {
		this->_error_pages[it->first] = ptr_copy[it->second];
	}
}

Location::~Location()
{
	std::map<int, std::string*>::iterator	it;
	std::set<std::string*>					pointers;
	std::set<std::string*>::iterator		pointer_it;

	for (it = this->_error_pages.begin(); it != this->_error_pages.end(); it++) {
		pointers.insert(it->second);
	}

	for (pointer_it = pointers.begin(); pointer_it != pointers.end(); pointer_it++)
		delete *pointer_it;
}

void Location::setAutoindex(const std::string& value)
{
	_autoindex = (value == "on" || value == "ON" || value == "true");
}

void Location::setMethods(const std::string& value)
{
	std::istringstream iss(value);
	std::string method;
	_methods.clear();
	while (iss >> method)
	{
		if (method != "GET" && method != "POST" && method != "DELETE")
			throw std::runtime_error("Method not supported");
		_methods.insert(method);
	}
}

void Location::setReturn(const std::string& value)
{
	std::istringstream iss(value);
	std::string first, second;

	iss >> first;
	if (iss >> second)
		_return = std::make_pair(std::atoi(first.c_str()), second);
	else
		_return = std::make_pair(301, first);
}

void Location::setIndex(const std::string& value)
{
	std::istringstream iss(value);
	std::string index;

	_index.clear();
	while (iss >> index)
	{
		_index.push_back(index);
	}
}

void Location::setAlias(const std::string& value)
{
	_alias = value;
}

void Location::setRoot(const std::string& value)
{
	if (value[0] == '~') {
		extern char** environ;
		for (char** env = environ; *env; ++env)
		{
			if (strncmp(*env, "HOME=", 5) == 0) {
				_root = (*env + 5) + value.substr(1);
				return ;
			}
		}
	}
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
	throw std::runtime_error("Error page configuration needs at least 2 values");
	}

	std::string* errorPathPtr = new std::string(words[words.size() - 1]);
	for (size_t i = 0; i < words.size() - 1; ++i)
	{
	std::istringstream converter(words[i]);
	int errorCode;

	if (!(converter >> errorCode) || !converter.eof())
	{
	throw std::runtime_error("Invalid error code: " + words[i]);
	}

	if (errorCode < 100 || errorCode > 599)
	{
	throw std::runtime_error("Error code out of range: " + words[i]);
	}

	_error_pages[errorCode] = errorPathPtr;
	}
}

void Location::setClientMaxBodySize(const std::string& value)
{
	std::string size = value;
	long multiplier = 1;

	if (size[size.length() - 1] == 'G' || size[size.length() - 1] == 'g')
	{
	multiplier = 1024 * 1024 * 1024;
	size.erase(size.size() - 1);
	}
	else if (size[size.length() - 1] == 'M' || size[size.length() - 1] == 'm')
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

const std::map<int, std::string*>& Location::getErrorPages(void) const
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

const std::string& Location::getAlias() const {
	return _alias;
}

const std::vector<std::string>& Location::getIndexes() const {
	return _index;
}

const std::map<std::string, std::string>& Location::getCGIs() const {
	return _cgis;
}

const std::pair<int, std::string>& Location::getRedirection() const {
	return _return;
}
