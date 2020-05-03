#include <iostream>
#include "commons.h"

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

int main()
{
    std::cout << "Hello World!\n";
}