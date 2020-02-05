//
// Created by Przemyslaw Podolski on 28/01/2020.
//

#ifndef TERMINAL_API_POC_HTTP_REQUEST_H
#define TERMINAL_API_POC_HTTP_REQUEST_H

#include <list>
#include <json/json.h>
#include <curl/curl.h>

#define DEBUG_CURL 0

class httpRequest
{
public:
    httpRequest(const char *url,
            std::map<std::string, std::string> &headers,
            std::string &data,
            int timeout);

    bool send();
    long getHttpResponseCode();
    std::stringstream &getHttpResponse();

private:
    std::string url;
    std::map<std::string, std::string> headers;
    std::string data;
    long httpResponseCode;
    std::stringstream httpResponse;
    int timeout;

private:
#ifdef DEBUG_CURL
    void curlDump(const char *text,
                 FILE *stream, unsigned char *ptr, size_t size,
                 char nohex);

    int curlTrace(CURL *handle, curl_infotype type,
                   char *data, size_t size,
                   void *userp);
#endif
};



#endif //TERMINAL_API_POC_HTTP_REQUEST_H
