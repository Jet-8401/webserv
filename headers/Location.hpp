#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <vector>

class Location {
	private:
		std::string					_root;
		std::vector<std::string>	_index;
		std::vector<std::string>	_methods;
};

#endif
