#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <set>

class Location {
	protected:
		std::set<std::string>	_methods;
		std::string				_root;
		std::string				_error_pages;
		bool					_autoindex;
		long					_client_max_body_size;

	public:
		const std::set<std::string>&	getMethods(void) const;
		const std::string&				getRoot(void) const;
		const std::string&				getErrorPages(void) const;
		const bool&						getAutoIndex(void) const;
		const long&						getClientMaxBodySize(void) const;
};

#endif
