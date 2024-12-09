bool	HttpHandler::_validateAndInitMethod(void)
{
	std::cout << "_validateAndInitMethod called" << std::endl;
	// only parse the headers
	if (!this->_parseHeaders()) {
		std::cout << "error while parsing headers" << std::endl;
		std::cout << "status_code: " << this->_status_code << std::endl;
		return (false);
	}

	// try to match a location
	if (!this->_findLocation()) {
		std::cout << "didn't find any locations" << std::endl;
		return (false);
	}

	std::cout << this->_config_location_str << " found!" << std::endl;
	std::cout << "Path requested: " << this->_path << std::endl;
	// check if method is allowed
	if (this->_matching_location->getMethods().find(this->_method) == this->_matching_location->getMethods().end())
		return (this->_status_code = 405, false);

	// based onto the headers check which extanded method to get
	if (this->_method == "GET") {
        // Vérifie d'abord si c'est un CGI
        std::string extension = this->_path.substr(this->_path.find_last_of("."));
        if (this->_matching_location->getCGIs().find(extension)
            != this->_matching_location->getCGIs().end()) {
            // this->_extanded_method = new HttpGetCGIMethod(*this);
            return true;
        }

        // Vérifie si c'est un répertoire
        if (this->_path[this->_path.length() - 1] == '/') {
            // if (this->_matching_location->getAutoIndex())
                // this->_extanded_method = new HttpGetDirectoryMethod(*this);
            // else
            //     this->_extanded_method = new HttpGetStaticMethod(*this); // cherchera index
            return true;
        }

        // Fichier statique par défaut
        this->_extanded_method = new HttpGetStaticMethod(*this);
    } else if (this->_method == "POST") {
    	std::cout << "POST" << std::endl;
        // this->_extanded_method = new HttpPostMethod(*this);
    } else if (this->_method == "DELETE") {
   	std::cout << "DELETE" << std::endl;
        // this->_extanded_method = new HttpDeleteMethod(*this);
    } else {
    	std::cout << "DELETE" << std::endl;
        this->_status_code = 501; // Not Implemented
        return false;
    }
	return (true);
}