//
// Created by Przemyslaw Podolski on 31/01/2020.
//

#ifndef TERMINAL_API_POC_ESCAPEJSON_H
#define TERMINAL_API_POC_ESCAPEJSON_H

#include <sstream>
#include <iomanip>

std::string escapeJSON(const std::string& input);
std::string unescapeJSON(const std::string& input);

#endif //TERMINAL_API_POC_ESCAPEJSON_H
