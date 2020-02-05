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
    enum transactionType { Sale, UnreferencedRefund, ReferencedRefund, CancelRefund, Cancel };

    paymentRequest(double amount,
            const char *currency,
            transactionType txType,
            integrationType iType,
            const char *refundTransactionId): amount(amount), currency(currency), txType(txType), iType(iType)
    {
        if (refundTransactionId)
        {
            this->refundTransactionId = refundTransactionId;
        }
        this->serviceId = generateServiceID();
        this->timestamp = generateTimestamp();
        this->endPoint = setEndPoint();
    };

    void send();
    void cancel();

    Json::Value &getPaymentResponse();

private:
    static std::string generateTimestamp();
    static std::string generateServiceID();
    std::string setEndPoint();

    void addMessageHeader(Json::Value &request);
    void addPaymentRequest(Json::Value &request);
    void addReversalRequest(Json::Value &request);
    void addAbortRequest(Json::Value &request);
    static void generateHttpHeaders(std::map<std::string, std::string> &httpHeaders);

    bool parseResponse(httpRequest &);

    double amount;
    std::string currency;
    transactionType txType;
    integrationType iType;
    std::string timestamp;
    std::string serviceId;
    std::string endPoint;
    std::string refundTransactionId;
    Json::Value jsonResponse;
};


#endif //TERMINAL_API_POC_PAYMENTREQUEST_H
