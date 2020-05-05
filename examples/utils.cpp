#include "utils.h"
#include <vector>

static CurlGlobalStateGuard handle_curl_state;

void curl_deinit(CURL* ptr)
{
    curl_easy_cleanup(ptr);
}

void curl_multi_deinit(CURLM* ptr)
{
    curl_multi_cleanup(ptr);
}

EasyHandle CreateEasyHandle()
{
    auto curl = EasyHandle(curl_easy_init(), curl_deinit);
    if (!curl)
    {
        throw std::runtime_error("Failed creating CURL easy object");
    }
    return curl;
}

MultiHandle CreateMultiHandle()
{
    auto curl = MultiHandle(curl_multi_init(), curl_multi_deinit);
    if (!curl)
    {
        throw std::runtime_error("Failed creating CURL multi object");
    }
    return curl;
}

void set_ssl(CURL* curl)
{
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
}

namespace
{
    size_t copy_to_memory(void* contents, size_t size, size_t nmemb, void* userp)
    {
        size_t realsize = size * nmemb;
        auto buf = *reinterpret_cast<std::vector<char>*>(userp);


        memcpy(buf.data(), contents, realsize);

        return realsize;
    }
}

void to_memory(CURL* curl)
{
    static std::vector<char> buf(1024 * 1024);
    /* send all data to this function  */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, copy_to_memory);
    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&buf));
}