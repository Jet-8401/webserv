#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <cstdlib>
# include <set>
# include <map>
# include <vector>

class Location {
	public:
		Location(void);
		Location(const Location& src);
		~Location(void);

		typedef	std::map<int, std::string*> error_page_t;
		typedef std::set<std::string>		methods_t;

		const std::set<std::string>&				getMethods(void) const;
		const std::string&							getRoot(void) const;
		const error_page_t&							getErrorPages(void) const;
		const bool&									getAutoIndex(void) const;
		const long&									getClientMaxBodySize(void) const;
		const std::string&							getAlias() const;
		const std::vector<std::string>&				getIndexes() const;
		const std::map<std::string, std::string>&	getCGIs() const;
		const std::pair<std::string, std::string>&	getRedirection() const;

		int setMethods(const std::string& value);
		int setRoot(const std::string& value);
		int setAlias(const std::string& value);
		int setErrorPage(const std::string& value);
		int setAutoindex(const std::string& value);
		int setClientMaxBodySize(const std::string& value);
		int setCgis(const std::string& value);
		int setIndex(const std::string& value);
		int setReturn(const std::string& value);

	protected:
		methods_t							_methods;
		std::string							_root;
		error_page_t						_error_pages;
		bool								_autoindex;
		long								_client_max_body_size;
		std::map<std::string, std::string>	_cgis;
		std::vector<std::string>			_index;
		std::pair<std::string, std::string>	_return;
		std::string							_alias;
};

#endif
