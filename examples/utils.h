#pragma once
#include <curl/curl.h>
#include <memory>
#include <functional>
#include <stdexcept>

using EasyHandle = std::unique_ptr<CURL, std::function<void(CURL*)>>;
using MultiHandle = std::unique_ptr<CURLM, std::function<void(CURLM*)>>;

EasyHandle CreateEasyHandle();
MultiHandle CreateMultiHandle();

int download_synchronous();
int download_asynchronous();
int download_multiplexing();
int download_synchronous_in_threads();

void set_ssl(CURL* curl);
void save_to_file(CURL* curl);

void multi_loop(CURLM* multi_handle);