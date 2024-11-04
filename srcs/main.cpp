#include "WebServ.hpp"

int	main(int argc, char* argv[])
{
	ServerCluster	cluster;

	if (argc != 2)
		return (error(ERR_USAGE), 0);
	if (cluster.loadConfig(argv[1]) == -1)
		return (1);
	return (0);
}
