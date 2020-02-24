//
// Created by Przemyslaw Podolski on 28/01/2020.
//

#ifndef TERMINAL_API_POC_TERMINALREQUEST_H
#define TERMINAL_API_POC_TERMINALREQUEST_H

#include <cstdint>
#include "httpRequest.h"
#include "POI.h"

class terminalRequest
{
public:
    enum integrationType { CloudSync, CloudAsync, Local, LocalEncrypted };
    enum requestType { Sale, UnreferencedRefund, ReferencedRefund, CancelRefund, Cancel, VirtualReceipt, Display, Barcode, Input };

    terminalRequest(double amount,
                    const char *currency,
                    requestType reqType,
                    integrationType iType,
                    const char *refundTransactionId = NULL): amount(amount), currency(currency), requestType(reqType), iType(iType)
    {
        if (refundTransactionId)
        {
            this->refundTransactionId = refundTransactionId;
        }
        this->serviceId = generateServiceID();
        this->timestamp = generateTimestamp();
        this->endPoint = setEndPoint();
    };

    terminalRequest(integrationType iType, const char *outputXHTML = NULL, requestType type = VirtualReceipt)
    {
        this->requestType = type;
        this->iType = iType;
        this->endPoint = setEndPoint();
        this->serviceId = generateServiceID();
        this->timestamp = generateTimestamp();
        this->displayData = outputXHTML;
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
    void addDisplayRequest(Json::Value &request, enum requestType type);

    static void generateHttpHeaders(std::map<std::string, std::string> &httpHeaders);

    bool parseResponse(httpRequest &);

    double amount;
    std::string currency;
    requestType requestType;
    integrationType iType;
    std::string timestamp;
    std::string serviceId;
    std::string endPoint;
    std::string refundTransactionId;
    std::string displayData;

    Json::Value jsonResponse;
};


#endif //TERMINAL_API_POC_TERMINALREQUEST_H
