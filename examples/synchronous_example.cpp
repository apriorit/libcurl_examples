#include <memory>
#include <iostream>
#include <list>
#include <algorithm>

#include "utils.h"

int download_synchronous(void)
{
    std::list<EasyHandle> handles(3);

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

    for (auto& handle : handles)
    {
        /* set options */
        curl_easy_setopt(handle.get(), CURLOPT_URL, "https://raw.githubusercontent.com/curl/curl/master/docs/examples/https.c"); 
        set_ssl(handle.get()); 
        save_to_file(handle.get()); 
        /* Perform the request, res will get the return code */
        auto res = curl_easy_perform(handle.get());
        /* Check for errors */
        if (res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed:" <<
                curl_easy_strerror(res) << std::endl;
            return -1;
        }
    }    

    return 0;
}