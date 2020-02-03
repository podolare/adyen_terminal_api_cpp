//
// Created by Przemyslaw Podolski on 28/01/2020.
//

#include "httpRequest.h"
#include <curl/curl.h>
#include <iostream>
#include <unistd.h>

#ifdef DEBUG_CURL
struct data
{
    char trace_ascii; /* 1 or 0 */
};

void httpRequest::curlDump(const char *text,
          FILE *stream, unsigned char *ptr, size_t size,
          char nohex)
{
    size_t i;
    size_t c;

    unsigned int width = 0x10;

    if(nohex)
        /* without the hex output, we can fit more on screen */
        width = 0x40;

    fprintf(stream, "%s, %10.10lu bytes (0x%8.8lx)\n",
            text, (unsigned long)size, (unsigned long)size);

    for(i = 0; i<size; i += width) {

        fprintf(stream, "%4.4lx: ", (unsigned long)i);

        if(!nohex) {
            /* hex not disabled, show it */
            for(c = 0; c < width; c++)
                if(i + c < size)
                    fprintf(stream, "%02x ", ptr[i + c]);
                else
                    fputs("   ", stream);
        }

        for(c = 0; (c < width) && (i + c < size); c++) {
            /* check for 0D0A; if found, skip past and start a new line of output */
            if(nohex && (i + c + 1 < size) && ptr[i + c] == 0x0D &&
               ptr[i + c + 1] == 0x0A) {
                i += (c + 2 - width);
                break;
            }
            fprintf(stream, "%c",
                    (ptr[i + c] >= 0x20) && (ptr[i + c]<0x80)?ptr[i + c]:'.');
            /* check again for 0D0A, to avoid an extra \n if it's at width */
            if(nohex && (i + c + 2 < size) && ptr[i + c + 1] == 0x0D &&
               ptr[i + c + 2] == 0x0A) {
                i += (c + 3 - width);
                break;
            }
        }
        fputc('\n', stream); /* newline */
    }
    fflush(stream);
}

int httpRequest::curlTrace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp)
{
    struct data *config = (struct data *)userp;
    const char *text;
    (void)handle; /* prevent compiler warning */

    switch(type) {
        case CURLINFO_TEXT:
            fprintf(stderr, "== Info: %s", data);
            /* FALLTHROUGH */
        default: /* in case a new one is introduced to shock us */
            return 0;

        case CURLINFO_HEADER_OUT:
            text = "=> Send header";
            break;
        case CURLINFO_DATA_OUT:
            text = "=> Send data";
            break;
        case CURLINFO_SSL_DATA_OUT:
            text = "=> Send SSL data";
            break;
        case CURLINFO_HEADER_IN:
            text = "<= Recv header";
            break;
        case CURLINFO_DATA_IN:
            text = "<= Recv data";
            break;
        case CURLINFO_SSL_DATA_IN:
            text = "<= Recv SSL data";
            break;
    }

    curlDump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
    return 0;
}
#endif

httpRequest::httpRequest(const char *url, std::map<std::string, std::string> &headers, std::string &data, int timeout, bool cloud)
{
    this->url = url;
    this->headers = headers;
    this->data = data;
    this->httpResponseCode = 0;
    this->timeout = timeout;
    this->cloud = cloud;
}

namespace
{
    std::size_t curlWriteCallback(
            const char* in,
            std::size_t size,
            std::size_t num,
            std::string* out)
    {
        std::string data(in, (std::size_t) size * num);
        *((std::stringstream*) out) << data;
        return size * num;
    }
}

bool httpRequest::send()
{
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();
    if(curl) {
        if (this->cloud)
        {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
            curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2 | CURL_SSLVERSION_MAX_DEFAULT);
        }
        else
        {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                curl_easy_setopt(curl, CURLOPT_CAPATH, cwd);
                curl_easy_setopt(curl, CURLOPT_CAINFO, "test.crt");
            }
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        }

#ifdef DEBUG_CURL
        struct data config;
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, &httpRequest::curlTrace);
        curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &config);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
        curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, this->timeout);
        //Add headers
        struct curl_slist *chunk = NULL;
        for (auto const& i : headers)
        {
            std::string tmp = i.first + ": " + i.second;
            chunk = curl_slist_append(chunk, tmp.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, this->data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, this->data.length());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &this->httpResponse);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &this->httpResponseCode);

        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed" << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "httpResponseCode: " << this->httpResponseCode << std::endl;
        }

        curl_slist_free_all(chunk);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return 0;
}

long httpRequest::getHttpResponseCode()
{
    return this->httpResponseCode;
}

std::stringstream &httpRequest::getHttpResponse()
{
    return httpResponse;
}