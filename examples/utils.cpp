#include "utils.h"
#include <vector>
#include <thread>
#include <iostream>

class CurlGlobalStateGuard
{
public:
    CurlGlobalStateGuard() { curl_global_init(CURL_GLOBAL_DEFAULT); }
    ~CurlGlobalStateGuard() { curl_global_cleanup(); }
};
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
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, copy_to_memory);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&buf));
}

void multi_loop(CURLM* multi_handle)
{
    int still_running = 0; /* keep number of running handles */

    /* we start some action by calling perform right away */
    curl_multi_perform(multi_handle, &still_running);

    while (still_running) {
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

        fd_set fdread;
        fd_set fdwrite;
        fd_set fdexcep;
        int maxfd = -1;
        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);
        /* get file descriptors from the transfers */
        auto mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

        if (mc != CURLM_OK) {
            std::cerr << "curl_multi_fdset() failed, code " << mc << "." << std::endl;
            break;
        }

        int rc = 0; /* select() return code */
        /* On success the value of maxfd is guaranteed to be >= -1. We call
           select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
           no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
           to sleep 100ms, which is the minimum suggested value in the
           curl_multi_fdset() doc. */

        if (maxfd == -1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else {
            /* Note that on some platforms 'timeout' may be modified by select().
               If you need access to the original value save a copy beforehand. */
            rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
        }

        if (rc >= 0)
        {
            /* timeout or readable/writable sockets */
            curl_multi_perform(multi_handle, &still_running);
        }
        /* else select error */
    }
}