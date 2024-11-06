#include "../headers/WebServ.hpp"

int	main(int argc, char* argv[])
{
	ServerCluster	cluster;

	if (argc != 2)
		return (error(ERR_USAGE, false), 0);
	if (cluster.importConfig(argv[1]) == -1)
		return (1);

	cluster.listenAll();
	return (0);
}
