#include <cstring>
#include <iostream>
#include <sys/socket.h>

int	main(void)
{
	int			fd;
	sockaddr_in	addr;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
		return (std::cerr << "Error" << std::endl, 0);

	memset(&addr, 0, sizeof(addr));
	addr.sa_family = AF_INET;
	addr.sin_addr.s_addr =
	if (bind(fd, &addr, sizeof(addr) == -1))
		return (std::cerr << "Error" << std::endl, 0);

	std::cout << "Success" << std::endl;
	return (0);
}
