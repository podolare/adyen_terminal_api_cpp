#include "paymentRequest.h"
#include "nexoCrypto.h"
#include "POI.h"
#include <unistd.h>
#include <thread>
#include <iostream>

void start_transaction(paymentRequest &request)
{
    request.send();
}

int main()
{
    nexoCrypto cryptoModule(POI::getCryptoPassphrase());

    paymentRequest request(
            109.9,
            "EUR",
            paymentRequest::Sale,
            paymentRequest::CloudSync,
            NULL);

    std::thread tx_thread{start_transaction, std::ref(request)};
    sleep(10);
    request.cancel();
    tx_thread.join();

    return 0;
}