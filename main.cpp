#include "paymentRequest.h"
#include "nexoCrypto.h"
#include "POI.h"
#include <unistd.h>

int main()
{
    nexoCrypto cryptoModule(POI::getCryptoPassphrase());

    paymentRequest request(
            109.9,
            "EUR",
            paymentRequest::Sale,
            paymentRequest::CloudAsync,
            NULL);

    request.send();
    sleep(2);
    request.cancel();

    return 0;
}