#include <iostream>

#include "token_scanner.h"

string_t TokenScanner::nextToken()
{
    if (_mode == multiple) {
        // Skip delimiter
        while (_current < _buffer.size() && _buffer[_current] == _delimiter) ++_current;
        int start = _current;

        // Find another delimiter
        while (_current < _buffer.size() && _buffer[_current] != _delimiter) ++_current;
        return _buffer.substr(start, _current - start);
    } else { // _mode == single
        int start = _current;

        // Find another delimiter
        while (_current < _buffer.size() && _buffer[_current] != _delimiter) ++_current;
        ++_current;
        return _buffer.substr(start, _current - start - 1);
    }
}

string_t TokenScanner::peekNextToken()
{
    if (_mode == multiple) {
        // Skip delimiter
        while (_current < _buffer.size() && _buffer[_current] == _delimiter) ++_current;
        int end = _current;

        // Find another delimiter
        while (end < _buffer.size() && _buffer[end] != _delimiter) ++end;
        return _buffer.substr(_current, end - _current);
    } else { // _mode == single
        int end = _current;

        // Find another delimiter
        while (end < _buffer.size() && _buffer[end] != _delimiter) ++end;
        return _buffer.substr(_current, end - _current);
    }
}

bool TokenScanner::hasMoreToken()
{
    if (_mode == multiple) {
        while (_current < _buffer.size() && _buffer[_current] == _delimiter) ++_current;
    }
    return _current != _buffer.size();
}

void TokenScanner::newLine()
{
    _current = 0;
    std::getline(std::cin, _buffer);
}

size_t TokenScanner::totalLength()
{
    return _buffer.length();
}

int stringToInt(const string_t& input)
{
    if (input.length() > 10) throw InvalidCommand("Invalid");
    if (input.length() == 10 && input > "2147483647") throw InvalidCommand("Invalid");
    int output = 0;
    for (const char c : input) {
        if (c < 48 || c > 57) throw InvalidCommand("Invalid");
        output = output * 10 + c - 48;
    }
    return output;
}

double stringToDouble(const string_t& input)
{
    double output = 0;
    bool point = false;
    double number = 1;
    for (const char c : input) {
        if (c < 46 || c == 47 || c > 57) throw InvalidCommand("Invalid");
        if (point) {
            if (c == '.') throw InvalidCommand("Invalid");
            number *= 10;
            output += double(c - 48) / number;
        } else {
            if (c == '.') {
                point = true;
                continue;
            }
            output = output * 10 + c - 48;
        }
    }
    return output;
}