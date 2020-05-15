#include "utils.h"
#include <vector>
#include <thread>
#include <iostream>
#include <fstream>
#include <string>
#include <tuple>
#include <openssl/engine.h>

class CurlGlobalStateGuard
{
public:
    CurlGlobalStateGuard() { curl_global_init(CURL_GLOBAL_DEFAULT); }
    ~CurlGlobalStateGuard() { curl_global_cleanup(); }
};
static CurlGlobalStateGuard handle_curl_state;

EasyHandle CreateEasyHandle()
{
    auto curl = EasyHandle(curl_easy_init(), curl_easy_cleanup);
    if (!curl)
    {
        throw std::runtime_error("Failed creating CURL easy object");
    }
    return curl;
}

MultiHandle CreateMultiHandle()
{
    auto curl = MultiHandle(curl_multi_init(), curl_multi_cleanup);
    if (!curl)
    {
        throw std::runtime_error("Failed creating CURL multi object");
    }
    return curl;
}

void set_ssl(CURL* curl)
{
    /*auto eng = ENGINE_by_id("dynamic");
    if (!eng)
    {
        ENGINE_load_dynamic();
        eng = ENGINE_by_id("dynamic");
    }
    std::cerr << eng << std::endl;
    auto name = eng ? ENGINE_get_id(eng) : "dynamic";
    auto res = curl_easy_setopt(curl, CURLOPT_SSLENGINE, name);
    curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);*/
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
}

namespace
{
size_t write_to_file(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    auto file = reinterpret_cast<std::ofstream*>(userp);
    file->write(reinterpret_cast<const char*>(contents), realsize);
    return realsize;
}
}

void save_to_file(CURL* curl)
{
    static std::ofstream file("downloaded_data.txt", std::ios::binary);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&file));
}

namespace
{
timeval get_timeout(CURLM* multi_handle)
{
    long curl_timeo = -1;
    /* set a suitable timeout to play around with */
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    curl_multi_timeout(multi_handle, &curl_timeo);
    if (curl_timeo >= 0) {
        timeout.tv_sec = curl_timeo / 1000;
        if (timeout.tv_sec > 1)
            timeout.tv_sec = 1;
        else
            timeout.tv_usec = (curl_timeo % 1000) * 1000;
    }
    return timeout;
}

int wait_if_needed(CURLM* multi_handle, timeval& timeout)
{
    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);
    int maxfd = -1;
    /* get file descriptors from the transfers */
    auto mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

    if (mc != CURLM_OK) {
        std::cerr << "curl_multi_fdset() failed, code " << mc << "." << std::endl;
    }
    /* On success the value of maxfd is guaranteed to be >= -1. We call
           sleep for 100ms, which is the minimum suggested value in the
           curl_multi_fdset() doc. */
    if (maxfd == -1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    int rc = maxfd != -1 ? select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout) : 0;
    return rc;
}
}

void multi_loop(CURLM* multi_handle)
{
    int still_running = 0; /* keep number of running handles */

    /* we start some action by calling perform right away */
    curl_multi_perform(multi_handle, &still_running);

    while (still_running) {
        struct timeval timeout = get_timeout(multi_handle);        
        
        auto rc = wait_if_needed(multi_handle, timeout);
        
        if (rc >= 0)
        {
            /* timeout or readable/writable sockets */
            curl_multi_perform(multi_handle, &still_running);
        }
        /* else select error */
    }
}