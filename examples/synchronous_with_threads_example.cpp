#include <memory>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <thread>

#include "utils.h"

int download_synchronous_in_threads(void)
{
    std::array<EasyHandle, 3> handles;

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
    std::for_each(handles.begin(), handles.end(), [](auto& handle) {
        curl_easy_setopt(handle.get(), CURLOPT_URL, "https://raw.githubusercontent.com/curl/curl/master/docs/examples/https.c");
        set_ssl(handle.get());
        save_to_file(handle.get());
    });

    std::vector<std::thread> threads(3);
    std::transform(handles.begin(), handles.end(), threads.begin(), [](EasyHandle& handle) {return std::thread([&handle]() {return curl_easy_perform(handle.get()); }); });
    std::for_each(threads.begin(), threads.end(), [](auto&& thread) {thread.join(); });   

    return 0;
}