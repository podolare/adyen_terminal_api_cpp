//
// Created by Przemyslaw Podolski on 28/01/2020.
//

#include "paymentRequest.h"
#include "nexoCrypto.h"
#include <json/json.h>
#include <iostream>
#include <time.h>


std::string paymentRequest::createTimestamp()
{
    char buffer[25] = {0};
    time_t tt;
    time(&tt);
    struct tm *tmp = localtime(&tt);
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", tmp);
    std::string timestamp(buffer);
    return timestamp;
}

std::string paymentRequest::generateServiceID()
{
    uint32_t iSecret;
    srand (time(NULL));
    /* generate secret number between 1 and 10: */
    iSecret = rand() % 9999999999 + 1;
    return std::to_string(iSecret);
}

void paymentRequest::send()
{
    bool cloud = false;
    const char *endPoint = nullptr;
    if (this->iType == CloudSync)
    {
        endPoint = POI::getSyncCloudUrl();
        cloud = true;
    }
    else if (this->iType == CloudAsync)
    {
        endPoint = POI::getAsyncCloudUrl();
        cloud = true;
    }
    else endPoint = POI::getTerminalAddress();

    std::map<std::string, std::string> headers;
    headers.insert(std::make_pair("x-API-key", POI::getXApiKey()));
    headers.insert(std::make_pair("Content-Type", "application/json"));

    this->jsonRequest["SaleToPOIRequest"]["MessageHeader"]["ProtocolVersion"] = "3.0";
    this->jsonRequest["SaleToPOIRequest"]["MessageHeader"]["MessageClass"] = "Service";
    this->jsonRequest["SaleToPOIRequest"]["MessageHeader"]["MessageCategory"] = "Payment";
    this->jsonRequest["SaleToPOIRequest"]["MessageHeader"]["MessageType"] = "Request";
    this->jsonRequest["SaleToPOIRequest"]["MessageHeader"]["SaleID"] = POI::getSaleId();
    std::string serviceId = this->generateServiceID();
    this->jsonRequest["SaleToPOIRequest"]["MessageHeader"]["ServiceID"] = serviceId; //needs to be unique
    this->jsonRequest["SaleToPOIRequest"]["MessageHeader"]["POIID"] = POI::getTerminalId();
    this->jsonRequest["SaleToPOIRequest"]["PaymentRequest"]["SaleData"]["SaleTransactionID"]["TransactionID"] =
            "MerchantRef" + serviceId;
    this->jsonRequest["SaleToPOIRequest"]["PaymentRequest"]["SaleData"]["SaleTransactionID"]["TimeStamp"] = this->createTimestamp();

    this->jsonRequest["SaleToPOIRequest"]["PaymentRequest"]["PaymentTransaction"]["AmountsReq"]["Currency"] = this->currency;
    this->jsonRequest["SaleToPOIRequest"]["PaymentRequest"]["PaymentTransaction"]["AmountsReq"]["RequestedAmount"] = this->amount;
    if (this->iType == LocalEncrypted) {
        nexoCrypto::encrypt(this->jsonRequest);
    }
    std::string requestData = this->jsonRequest.toStyledString();
    std::cout << "Request:\n" << requestData.c_str() << std::endl;

    httpRequest request(endPoint, headers, requestData, POI::getDefaultTimeout(), cloud);
    request.send();

    if (request.getHttpResponseCode() != 0)
    {
        this->parseResponse(request);
    }
}

bool paymentRequest::parseResponse(httpRequest &request)
{
    Json::CharReaderBuilder jsonReader;
    std::string parseError;
    Json::Value response;
    if (Json::parseFromStream(jsonReader, request.getHttpResponse(), &response, &parseError))
    {
        if (this->iType == LocalEncrypted)
        {
            this->jsonResponse = nexoCrypto::decrypt(response);
        }
        else
        {
            this->jsonResponse = response;
        }
        std::cout << "Response:\n" << this->jsonResponse.toStyledString() << std::endl;
        return true;
    }
    else
    {
        std::cout << "Could not parse HTTP data as JSON" << parseError << std::endl;
    }
    return false;
}

Json::Value &paymentRequest::getPaymentResponse()
{
    return jsonResponse;
}