#include <memory>
#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>

#include "utils.h"

int download_synchronous_in_threads(void)
{
    std::vector<EasyHandle> handles(3);

    /* init easy stacks */
    try
    {
        std::for_each(handles.begin(), handles.end(), [](auto& handle) {handle = CreateEasyHandle(); });
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

    std::vector<std::thread> threads(3);
    std::transform(handles.begin(), handles.end(), threads.begin(), [](EasyHandle& handle) {return std::thread([&handle]() {return curl_easy_perform(handle.get()); }); });
    std::for_each(threads.begin(), threads.end(), [](auto&& thread) {thread.join(); });   

    return 0;
}