#include "paymentRequest.h"
#include "nexoCrypto.h"
#include "POI.h"

int main()
{
    nexoCrypto cryptoModule(POI::getCryptoPassphrase());

    paymentRequest request(109.9, "GBP", paymentRequest::Sale, paymentRequest::LocalEncrypted);

    request.send();

    return 0;
}