#include "../headers/ServerCluster.hpp"
#include <cstdio>
#include <fstream>
#include <stdexcept>

// Constructors / Destructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

ServerCluster::ServerCluster(const std::string& configPath)
{
	this->importConfig(configPath);
}

ServerCluster::~ServerCluster(void)
{}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

void	ServerCluster::importConfig(const std::string& configPath)
{
	std::ifstream	configFile(configPath);

	if (!configFile.good())
		throw std::runtime_error(ERR_FILE_OPEN + configPath);

	// parse json
}
