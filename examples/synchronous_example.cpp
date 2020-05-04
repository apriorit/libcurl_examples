#include <memory>
#include <iostream>
#include <vector>
#include <algorithm>

#include "commons.h"

int download_synchronous(void)
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
    curl_easy_setopt(handles[0].get(), CURLOPT_URL, "http://www.example.com/");
    curl_easy_setopt(handles[1].get(), CURLOPT_URL, "http://localhost/");
    curl_easy_setopt(handles[2].get(), CURLOPT_URL, "http://google.com/");

    for (auto& handle : handles)
    {
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