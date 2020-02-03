//
// Created by Przemyslaw Podolski on 31/01/2020.
//

#include "escapeJson.h"

enum State {ESCAPED, UNESCAPED};

std::string escapeJSON(const std::string& input)
{
    std::string output;
    output.reserve(input.length());

    for (std::string::size_type i = 0; i < input.length(); ++i)
    {
        switch (input[i]) {
            case '\"':
            case '/':
            case '\\':
                output += input[i];
                break;
            case '\b':
                output += 0x08;
                break;
            case '\f':
                output += 0x0c;
                break;
            case '\n':
                output += 0x0a;
                break;
            case '\r':
                output += 0x0d;
                break;
            case '\t':
                output += 0x09;
                break;
            default:
                output += input[i];
                break;
        }
    }

    return output;
}

std::string unescapeJSON(const std::string& input)
{
    State s = UNESCAPED;
    std::string output;
    output.reserve(input.length());

    for (std::string::size_type i = 0; i < input.length(); ++i)
    {
        switch(s)
        {
            case ESCAPED:
            {
                switch(input[i])
                {
                    case '"':
                        output += '\"';
                        break;
                    case '/':
                        output += '/';
                        break;
                    case 'b':
                        output += '\b';
                        break;
                    case 'f':
                        output += '\f';
                        break;
                    case 'n':
                        output += '\n';
                        break;
                    case 'r':
                        output += '\r';
                        break;
                    case 't':
                        output += '\t';
                        break;
                    case '\\':
                        output += '\\';
                        break;
                    default:
                        output += input[i];
                        break;
                }

                s = UNESCAPED;
                break;
            }
            case UNESCAPED:
            {
                switch(input[i])
                {
                    case '\\':
                        s = ESCAPED;
                        break;
                    default:
                        output += input[i];
                        break;
                }
            }
        }
    }
    return output;
}