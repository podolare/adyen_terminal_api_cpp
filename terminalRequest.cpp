//
// Created by Przemyslaw Podolski on 28/01/2020.
//

#include "terminalRequest.h"
#include "nexoCrypto.h"
#include <json/json.h>
#include <iostream>
#include <random>

void terminalRequest::addMessageHeader(Json::Value &request)
{
    request["SaleToPOIRequest"]["MessageHeader"]["ProtocolVersion"] = "3.0";
    request["SaleToPOIRequest"]["MessageHeader"]["MessageClass"] = "Service";
    request["SaleToPOIRequest"]["MessageHeader"]["ServiceID"] = this->serviceId; //needs to be unique
    std::string messageCat{"Payment"};
    if (this->requestType == ReferencedRefund) {
        messageCat = "Reversal";
    } else if (this->requestType == Cancel) {
        messageCat = "Abort";
        //for Cancel new service id in the header is needed
        request["SaleToPOIRequest"]["MessageHeader"]["ServiceID"] = generateServiceID();
    } else if (this->requestType == Barcode || this->requestType == VirtualReceipt) {
        messageCat = "Display";
    }
    request["SaleToPOIRequest"]["MessageHeader"]["MessageCategory"] = messageCat;
    request["SaleToPOIRequest"]["MessageHeader"]["MessageType"] = "Request";
    request["SaleToPOIRequest"]["MessageHeader"]["SaleID"] = POI::getSaleId();

    request["SaleToPOIRequest"]["MessageHeader"]["POIID"] = POI::getTerminalId();
}

void terminalRequest::addPaymentRequest(Json::Value &request)
{
    request["SaleToPOIRequest"]["PaymentRequest"]["SaleData"]["SaleTransactionID"]["TransactionID"] = "MerchantReference" + this->serviceId;
    request["SaleToPOIRequest"]["PaymentRequest"]["SaleData"]["SaleTransactionID"]["TimeStamp"] = this->timestamp;
    request["SaleToPOIRequest"]["PaymentRequest"]["PaymentTransaction"]["AmountsReq"]["Currency"] = this->currency;
    request["SaleToPOIRequest"]["PaymentRequest"]["PaymentTransaction"]["AmountsReq"]["RequestedAmount"] = this->amount;
    if (this->requestType == UnreferencedRefund)
    {
        request["SaleToPOIRequest"]["PaymentRequest"]["PaymentData"]["PaymentType"] = "Refund";
    }
}

void terminalRequest::addAbortRequest(Json::Value &request)
{
    request["SaleToPOIRequest"]["AbortRequest"]["MessageReference"]["MessageCategory"] = "Payment";
    request["SaleToPOIRequest"]["AbortRequest"]["MessageReference"]["SaleID"] = POI::getSaleId();
    request["SaleToPOIRequest"]["AbortRequest"]["MessageReference"]["ServiceID"] = this->serviceId;
    request["SaleToPOIRequest"]["AbortRequest"]["AbortReason"] = "MerchantAbort";
}

void terminalRequest::addReversalRequest(Json::Value &request)
{
    request["SaleToPOIRequest"]["ReversalRequest"]["OriginalPOITransaction"]["POITransactionID"]["TransactionID"] = this->refundTransactionId;
    request["SaleToPOIRequest"]["ReversalRequest"]["OriginalPOITransaction"]["POITransactionID"]["TimeStamp"] = this->timestamp;
    request["SaleToPOIRequest"]["ReversalRequest"]["ReversalReason"] = "MerchantCancel";
}

void terminalRequest::addDisplayRequest(Json::Value &request, enum requestType type)
{
    Json::Value displayOutput;
    displayOutput["DisplayOutput"][0]["InfoQualify"] = "Display";
    displayOutput["DisplayOutput"][0]["Device"] = "CustomerDisplay";
    if (type == VirtualReceipt) {
        displayOutput["DisplayOutput"][0]["OutputContent"]["OutputXHTML"] = this->displayData;
        displayOutput["DisplayOutput"][0]["OutputContent"]["OutputFormat"] = "XHTML";
    } else if (type == Barcode) {
        displayOutput["DisplayOutput"][0]["OutputContent"]["OutputFormat"] = "BarCode";
        displayOutput["DisplayOutput"][0]["OutputContent"]["OutputBarcode"]["BarcodeValue"] = this->displayData;
        displayOutput["DisplayOutput"][0]["OutputContent"]["OutputBarcode"]["BarcodeType"] = "QRCode";
    }
    request["SaleToPOIRequest"]["DisplayRequest"] = displayOutput;
    std::cout << "Request:\n" << request.toStyledString() << std::endl;
}

void terminalRequest::generateHttpHeaders(std::map<std::string, std::string> &httpHeaders)
{
    httpHeaders.insert(std::make_pair("x-API-key", POI::getXApiKey()));
    httpHeaders.insert(std::make_pair("Content-Type", "application/json"));
}

void terminalRequest::send()
{
    Json::Value jsonRequest;
    addMessageHeader(jsonRequest);
    if (this->requestType == ReferencedRefund) {
        addReversalRequest(jsonRequest);
    } else if (this->requestType == VirtualReceipt || this->requestType == Barcode) {
        addDisplayRequest(jsonRequest, this->requestType);
    } else {
        addPaymentRequest(jsonRequest);
    }
    std::cout << "Request:\n" << jsonRequest.toStyledString() << std::endl;
    if (this->iType == LocalEncrypted) {
        nexoCrypto::encrypt(jsonRequest);
    }
    std::map<std::string, std::string> httpHeaders;
    generateHttpHeaders(httpHeaders);
    std::string requestData = jsonRequest.toStyledString();
    httpRequest request(this->endPoint.c_str(), httpHeaders, requestData, POI::getDefaultTimeout());
    request.send();

    if (request.getHttpResponseCode() == 200 && this->iType != CloudAsync)
    {
        this->parseResponse(request);
    }
}

void terminalRequest::cancel()
{
    this->requestType = Cancel;
    Json::Value jsonRequest;
    addMessageHeader(jsonRequest);
    addAbortRequest(jsonRequest);
    std::cout << "Cancel request:\n" << jsonRequest.toStyledString() << std::endl;
    if (this->iType == LocalEncrypted) {
        nexoCrypto::encrypt(jsonRequest);
    }
    std::map<std::string, std::string> httpHeaders;
    generateHttpHeaders(httpHeaders);
    std::string requestData = jsonRequest.toStyledString();
    httpRequest request(this->endPoint.c_str(), httpHeaders, requestData, POI::getDefaultTimeout());
    request.send();
    if (request.getHttpResponseCode() == 200 && this->iType != CloudAsync)
    {
        this->parseResponse(request);
    }
}

bool terminalRequest::parseResponse(httpRequest &request)
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
        std::cout << "Could not parse HTTP data as JSON" << parseError << std::endl
                    << "Data: \n" << request.getHttpResponse().str() << std::endl;
    }
    return false;
}

std::string terminalRequest::generateTimestamp() {
    char buffer[25] = {0};
    time_t tt;
    time(&tt);
    struct tm *tmp = localtime(&tt);
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", tmp);
    std::string timestamp(buffer);
    return timestamp;
}

std::string terminalRequest::generateServiceID() {
    std::random_device r;
    std::default_random_engine e1(r());
    std::uniform_int_distribution<int> uniform_dist(1, 999999999);
    int mean = uniform_dist(e1);
    return std::to_string(mean);
}

std::string terminalRequest::setEndPoint() {
    std::ostringstream strbuf;
    if (this->iType == CloudSync) {
        strbuf << POI::getSyncCloudUrl();
    } else if (this->iType == CloudAsync) {
        strbuf << POI::getAsyncCloudUrl();
    } else {
        strbuf << POI::getTerminalAddress();
    }

    if (this->requestType == ReferencedRefund || this->requestType == CancelRefund) {
        if (this->requestType == CancelRefund) {
            strbuf << "/voidPendingRefund";
        }
    }
    return strbuf.str();
}

Json::Value &terminalRequest::getPaymentResponse()
{
    return jsonResponse;
}