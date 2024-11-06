#include "../headers/ServerCluster.hpp"
#include <cstdio>
#include <unistd.h>

// Constructors / Destructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

ServerCluster::ServerCluster(void)
{}

ServerCluster::~ServerCluster(void)
{
	close(this->_epoll_fd);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

int	ServerCluster::importConfig(const std::string& configPath)
{

	return (0);
}

int	ServerCluster::listenAll(void) const
{
	servers_type_t::const_iterator	it;

	for (it = this->_servers.begin(); it != this->_servers.end(); it++)
		if ((*it).second.listen() == -1)
			return (-1);
	return (0);
}
