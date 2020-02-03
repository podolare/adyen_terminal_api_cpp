//
// Created by Przemyslaw Podolski on 29/01/2020.
//

#include "POI.h"

const char POI::xapikey[] = "AQEthmfxKYvNahVBw0m/n3Q5qf3VfI5eGbBFVXVXyGEKf29LI1qIaEE6HPVhwFTrEMFdWw2+5HzctViMSCJMYAc=-iteHlnC3rcUTit3pyO7lRwFCb1bXF6Jzt9QvlY387CM=-Ky6qQ9Ws33uD67PE";
const char POI::terminalId[] = "P400Plus-275103190";
//const char POI::terminalId[] = "P400Plus-275086634";
const char POI::saleId[] = "CashRegisterId";
const char POI::terminalAddress[] = "https://172.17.125.215:8443/nexo";
//const char POI::terminalAddress[] = "https://172.17.126.232:8443/nexo";
const char POI::syncCloudUrl[] = "https://terminal-api-test.adyen.com/sync";
const char POI::asyncCloudUrl[] = "https://terminal-api-test.adyen.com/async";
const char POI::cryptoPassphrase[] = "mysupersecretpassphrase";
const char POI::keyIdentifier[] = "mykey";
const int POI::keyVersion = 0;
const int POI::adyenCrypto = 1;
const int POI::defaultTimeout = 120000; //miliseconds
