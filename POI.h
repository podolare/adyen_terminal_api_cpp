//
// Created by Przemyslaw Podolski on 29/01/2020.
//

#ifndef TERMINAL_API_POC_POI_H
#define TERMINAL_API_POC_POI_H

class POI
{
public:
    static const char *getXApiKey() { return xapikey; }
    static const char *getCryptoPassphrase() { return cryptoPassphrase; }
    static const char *getTerminalId() { return terminalId; }
    static const char *getSaleId() { return saleId; }
    static const char *getTerminalAddress() { return terminalAddress; }
    static const char *getSyncCloudUrl() { return syncCloudUrl; }
    static const char *getAsyncCloudUrl() { return asyncCloudUrl; }
    static const char *getKeyIdentifier() { return keyIdentifier; }
    static const int getKeyVersion() { return keyVersion; }
    static const int getAdyenCryptoVersion() { return adyenCrypto; }
    static const int getDefaultTimeout() { return defaultTimeout; }
private:
    static const char xapikey[];
    static const char terminalId[];
    static const char saleId[];
    static const char terminalAddress[];
    static const char syncCloudUrl[];
    static const char asyncCloudUrl[];
    static const char cryptoPassphrase[];
    static const char keyIdentifier[];
    static const int keyVersion;
    static const int adyenCrypto;
    static const int defaultTimeout;
};


#endif //TERMINAL_API_POC_POI_H
