#include <iostream>
#include <memory>
#include <array>
#include <algorithm>
#include <thread>

#include "utils.h"

/*
 * Download many transfers over HTTP/2, using the same connection!
 */
int download_multiplexing(void)
{
    std::array<EasyHandle, 3> handles;
    MultiHandle multi_handle;

    /* init easy and multi stacks */
    try
    {
        multi_handle = CreateMultiHandle();
        for(auto& handle : handles)
        {
            handle = CreateEasyHandle();
            /* HTTP/2 please */
            curl_easy_setopt(handle.get(), CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
            /* wait for pipe connection to confirm */
            curl_easy_setopt(handle.get(), CURLOPT_PIPEWAIT, 1L);
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

    multi_loop(multi_handle.get());
    
    std::for_each(handles.begin(), handles.end(), [&multi_handle](auto& handle) {curl_multi_remove_handle(multi_handle.get(), handle.get()); });
    return 0;
}