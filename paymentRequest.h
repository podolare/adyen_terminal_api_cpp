//
// Created by Przemyslaw Podolski on 28/01/2020.
//

#ifndef TERMINAL_API_POC_PAYMENTREQUEST_H
#define TERMINAL_API_POC_PAYMENTREQUEST_H

#include <cstdint>
#include "httpRequest.h"
#include "POI.h"

class paymentRequest
{
public:
    enum integrationType { CloudSync, CloudAsync, Local, LocalEncrypted };
    enum transactionType { Sale, Refund };

    paymentRequest(double amount,
            const char *currency,
            transactionType txType,
            integrationType iType) : amount(amount), currency(currency), txType(txType), iType(iType) {};

    void send();

    Json::Value &getPaymentResponse();

private:
    double amount;
    std::string currency;
    transactionType txType;
    integrationType iType;

    Json::Value jsonRequest;
    Json::Value jsonResponse;

    bool parseResponse(httpRequest &);

    std::string createTimestamp();
    std::string generateServiceID();
};


#endif //TERMINAL_API_POC_PAYMENTREQUEST_H
