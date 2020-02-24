#include "terminalRequest.h"
#include "nexoCrypto.h"
#include "POI.h"
#include <unistd.h>
#include <thread>
#include <iostream>

void start_transaction(terminalRequest &request)
{
    request.send();
}

int main()
{
    nexoCrypto cryptoModule(POI::getCryptoPassphrase());

//    terminalRequest request(
//            109.9,
//            "EUR",
//            terminalRequest::Sale,
//            terminalRequest::LocalEncrypted,
//            NULL);

    terminalRequest request(terminalRequest::LocalEncrypted, POI::getBarcodeRequestData(), terminalRequest::Barcode);
    request.send();

//    std::thread tx_thread{start_transaction, std::ref(request)};
//    sleep(10);
//    request.cancel();
//    tx_thread.join();

    return 0;
}