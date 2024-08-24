#pragma once

#include <string>

struct ServerInfo
{
    std::string host;
    std::string path;
    bool ssl;
    bool break_on_success;

    std::string get_url(std::string id) const
    {
        std::string url = ssl ? "https://" : "http://";
        url += host;
        if (!path.empty())
        {
            url += ("/" + path);
        }
        url += ("/" + id);
        return  url;
    }
};