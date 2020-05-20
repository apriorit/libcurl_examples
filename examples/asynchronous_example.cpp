#include <iostream>
#include <memory>
#include <list>
#include <algorithm>

#include "utils.h"

int download_asynchronous(void)
{
    std::list<EasyHandle> handles(3);
    MultiHandle multi_handle;

    /* init easy and multi stacks */
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
    std::for_each(handles.begin(), handles.end(), [](auto& handle) {
        curl_easy_setopt(handle.get(), CURLOPT_URL, "https://raw.githubusercontent.com/curl/curl/master/docs/examples/multi-double.c");
        set_ssl(handle.get());
        save_to_file(handle.get());
    });

    /* add the individual transfers */
    std::for_each(handles.begin(), handles.end(), [&multi_handle](auto& handle) {curl_multi_add_handle(multi_handle.get(), handle.get()); });

    multi_loop(multi_handle.get());

    std::for_each(handles.begin(), handles.end(), [&multi_handle](auto& handle) {curl_multi_remove_handle(multi_handle.get(), handle.get()); });
    return 0;
}