#include "../headers/ServerConfig.hpp"
#include "../headers/HttpServer.hpp"
#include "../headers/ServerCluster.hpp"

int main(int argc, char* argv[])
{
	(void) argc;
	(void) argv;

	ServerConfig	configuration;
	configuration.setHost("0.0.0.0");
	configuration.setPort("8003");
	configuration.setServerName("jullopez.42.fr");

	HttpServer	server(configuration);
	ServerCluster	cluster;
	cluster.addServer(server);
	cluster.listenAll();

    return 0;
}
