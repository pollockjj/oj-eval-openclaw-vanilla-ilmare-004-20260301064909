#ifndef TOKEN_SCANNER
#define TOKEN_SCANNER

#include <string>
#include <utility>

#include "exception.h"

typedef char char_t;
typedef std::string string_t;

int stringToInt(const string_t& input);

double stringToDouble(const string_t& input);

class TokenScanner {
public:
    enum tokenScannerMode {multiple, single};

private:
    string_t _buffer;

    char_t _delimiter = ' ';

    int _current = 0;

    tokenScannerMode _mode = multiple;

public:

    TokenScanner() = default;

    ~TokenScanner() = default;

    explicit TokenScanner(string_t input, char_t delimiter = ' ', tokenScannerMode mode = multiple)
    : _buffer(std::move(input) + delimiter), _delimiter(delimiter), _mode(mode) {}

    void newLine();

    string_t nextToken();

    string_t peekNextToken();

    size_t totalLength();

    bool hasMoreToken();
};

#endif //TOKEN_SCANNER
