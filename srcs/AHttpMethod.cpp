#include "../headers/AHttpMethod.hpp"
#include "../headers/HttpHandler.hpp"
#include "../headers/WebServ.hpp"
#include <cstring>

AHttpMethod::AHttpMethod(HttpHandler& handler) :
    referer(handler),
    _headers_sent(false)
{
    ::memset(&this->_file_stat, 0, sizeof(this->_file_stat));
}

AHttpMethod::~AHttpMethod(void)
{}

bool AHttpMethod::_resolveLocation(void)
{
    // Construction du chemin complet
    this->_complete_path = joinPath(
        this->referer.getMatchingLocation()->getRoot(),
        this->referer.getPath()
    );

    // Gestion de l'alias si présent
    const std::string& alias = this->referer.getMatchingLocation()->getAlias();
    if (!alias.empty()) {
        std::string loc_str = this->referer.getLocationString();
        this->_complete_path.replace(
            this->_complete_path.find(loc_str),
            loc_str.length(),
            alias
        );
    }

    // Vérification de l'existence
    if (::stat(this->_complete_path.c_str(), &this->_file_stat) == -1) {
        this->referer.setStatusCode(404);
        return false;
    }

    return true;
}
