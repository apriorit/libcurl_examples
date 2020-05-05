#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <thread>

#include "utils.h"

#ifndef CURLPIPE_MULTIPLEX
/* This little trick will just make sure that we don't enable pipelining for
   libcurls old enough to not have this symbol. It is _not_ defined to zero in
   a recent libcurl header. */
#define CURLPIPE_MULTIPLEX 0
#endif

/*
 * Download many transfers over HTTP/2, using the same connection!
 */
int download_multiplexing(void)
{
    std::vector<EasyHandle> handles(3);
    MultiHandle multi_handle;

    /* init easy stacks */
    try
    {
        multi_handle = CreateMultiHandle();
        for(auto& handle : handles)
        {
            handle = CreateEasyHandle();
            /* HTTP/2 please */
            curl_easy_setopt(handle.get(), CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
#if (CURLPIPE_MULTIPLEX > 0)
            /* wait for pipe connection to confirm */
            curl_easy_setopt(handle.get(), CURLOPT_PIPEWAIT, 1L);
#endif
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return -1;
    }
    /* set options */
    curl_easy_setopt(handles[0].get(), CURLOPT_URL, "https://curl.haxx.se/libcurl/c/https.html");
    curl_easy_setopt(handles[1].get(), CURLOPT_URL, "https://curl.haxx.se/libcurl/c/multi-double.html");
    curl_easy_setopt(handles[2].get(), CURLOPT_URL, "https://curl.haxx.se/libcurl/c/http2-download.html");
    std::for_each(handles.begin(), handles.end(), [](auto& handle) {
        set_ssl(handle.get());
        to_memory(handle.get());
    });

    /* add the individual transfers */
    std::for_each(handles.begin(), handles.end(), [&multi_handle](auto& handle) {curl_multi_add_handle(multi_handle.get(), handle.get()); });

    curl_multi_setopt(multi_handle.get(), CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);

    int still_running = 0; /* keep number of running handles */
    /* we start some action by calling perform right away */
    curl_multi_perform(multi_handle.get(), &still_running);

    while (still_running) {
        long curl_timeo = -1;
        /* set a suitable timeout to play around with */
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        curl_multi_timeout(multi_handle.get(), &curl_timeo);
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
        auto mc = curl_multi_fdset(multi_handle.get(), &fdread, &fdwrite, &fdexcep, &maxfd);

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
            curl_multi_perform(multi_handle.get(), &still_running);
        }
        /* else select error */
    }
    std::for_each(handles.begin(), handles.end(), [&multi_handle](auto& handle) {curl_multi_remove_handle(multi_handle.get(), handle.get()); });
    return 0;
}