#include <memory>
#include <iostream>
#include <vector>

#include "commons.h"

int download_synchronous(void)
{
    std::vector<EasyHandle> handles(3);

    /* init easy stacks */
    try
    {
        handles[0] = CreateEasyHandle();
        handles[1] = CreateEasyHandle();
        handles[2] = CreateEasyHandle();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return -1;
    }
    /* set options */
    curl_easy_setopt(handles[0].get(), CURLOPT_URL, "https://www.example.com/");
    curl_easy_setopt(handles[1].get(), CURLOPT_URL, "http://localhost/");
    curl_easy_setopt(handles[2].get(), CURLOPT_URL, "http://google.com/");

#ifdef SKIP_PEER_VERIFICATION
    /*
        * If you want to connect to a site who isn't using a certificate that is
        * signed by one of the certs in the CA bundle you have, you can skip the
        * verification of the server's certificate. This makes the connection
        * A LOT LESS SECURE.
        *
        * If you have a CA cert for the server stored someplace else than in the
        * default bundle, then the CURLOPT_CAPATH option might come handy for
        * you.
        */
    curl_easy_setopt(handles[0].get(), CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(handles[1].get(), CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(handles[2].get(), CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
        * If the site you're connecting to uses a different host name that what
        * they have mentioned in their server certificate's commonName (or
        * subjectAltName) fields, libcurl will refuse to connect. You can skip
        * this check, but this will make the connection less secure.
        */
    curl_easy_setopt(handles[0].get(), CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(handles[1].get(), CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(handles[2].get(), CURLOPT_SSL_VERIFYHOST, 0L);
#endif

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