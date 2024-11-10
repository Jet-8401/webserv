#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <cstdlib>
# include <set>
# include <map>

class Location {
	protected:
		std::set<std::string>				_methods;
		std::string							_root;
		std::map<std::string, std::string*>	_error_pages;
		bool								_autoindex;
		long								_client_max_body_size;

	public:
		Location(void);

		const std::set<std::string>&				getMethods(void) const;
		const std::string&							getRoot(void) const;
		const std::map<std::string, std::string*>&	getErrorPages(void) const;
		const bool&									getAutoIndex(void) const;
		const long&									getClientMaxBodySize(void) const;

        void setAutoindex(const std::string& value);
        void setMethods(const std::string& value);
        void setRoot(const std::string& value);
        void setErrorPage(const std::string& value);
        void setClientMaxBodySize(const std::string& value);
};

#endif
