#include "../headers/HttpDeleteRequest.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <ftw.h>

static int removeCallback(const char *path, const struct stat *stat, int flag, struct FTW *ftw)
{
    (void)stat;
    (void)ftw;
    if (flag == FTW_DP)
        return rmdir(path);
    return unlink(path);
}

HttpDeleteRequest::HttpDeleteRequest(const ServerConfig& config)
    : HttpRequest(config), _is_directory(false)
{}

HttpDeleteRequest::~HttpDeleteRequest()
{}

bool HttpDeleteRequest::parse(const uint8_t* packet, const size_t packet_size)
{
    if (!HttpRequest::parse(packet, packet_size))
        return false;

    if (this->_state == CHECK_METHOD) {
        if (!_checkPath())
            return false;

        if (_is_directory) {
            if (!_deleteDirectory())
                return false;
        } else {
            if (!_deleteFile())
                return false;
        }

        this->_state = DONE;
    }

    return true;
}

bool HttpDeleteRequest::_checkPath()
{
    _target_path = _matching_location->getRoot() + _path;

    struct stat path_stat;
    if (stat(_target_path.c_str(), &path_stat) != 0)
        return false;

    _is_directory = S_ISDIR(path_stat.st_mode);
    return true;
}

bool HttpDeleteRequest::_deleteFile()
{
    return unlink(_target_path.c_str()) == 0;
}

bool HttpDeleteRequest::_deleteDirectory()
{
    return nftw(_target_path.c_str(), removeCallback, 64, FTW_DEPTH | FTW_PHYS) == 0;
}

ssize_t HttpDeleteRequest::writePacket(uint8_t* io_buffer, size_t buff_length)
{
    // DELETE requests typically don't need to write response packets
    // Response handling should be done by the response class
    (void)io_buffer;
    (void)buff_length;
    return 0;
}
