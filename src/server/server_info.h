#pragma once

#include <string>

struct ServerInfo
{
    std::string host;
    std::string path;
    bool ssl;
    bool break_on_success;
    int offset;

    std::string get_url(int id) const
    {
        std::string url = ssl ? "https://" : "http://";
        url += host;
        if (!path.empty())
        {
            url += ("/" + path);
        }
        url += ("/" + std::to_string(id + offset));
        return  url;
    }
};