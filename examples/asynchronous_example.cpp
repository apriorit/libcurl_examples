#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

#include "utils.h"

int download_asynchronous(void)
{
    std::vector<EasyHandle> handles(1000);
    MultiHandle multi_handle;

    /* init easy stacks */
    try
    {
        multi_handle = CreateMultiHandle();
        std::for_each(handles.begin(), handles.end(), [](auto& handle){handle = CreateEasyHandle(); });
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

    multi_loop(multi_handle.get());

    std::for_each(handles.begin(), handles.end(), [&multi_handle](auto& handle) {curl_multi_remove_handle(multi_handle.get(), handle.get()); });
    return 0;
}