#include <iostream>
#include <memory>
#include <list>
#include <algorithm>
#include <thread>

#include "utils.h"

/*
 * Download many transfers over HTTP/2, using the same connection!
 */
int download_multiplexing(void)
{
    std::list<EasyHandle> handles(3);
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

    for (auto& handle : handles)
    {
        curl_easy_setopt(handle.get(), CURLOPT_URL, "https://curl.haxx.se/libcurl/c/http2-download.html");
        set_ssl(handle.get());
        save_to_file(handle.get());
        /* add the individual transfers */
        curl_multi_add_handle(multi_handle.get(), handle.get());
    }

    curl_multi_setopt(multi_handle.get(), CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);

    multi_loop(multi_handle.get());
    
    std::for_each(handles.begin(), handles.end(), [&multi_handle](auto& handle) {curl_multi_remove_handle(multi_handle.get(), handle.get()); });
    return 0;
}