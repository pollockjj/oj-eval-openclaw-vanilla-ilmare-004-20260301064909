#ifndef EXCEPTION
#define EXCEPTION

#include <exception>

class InvalidCommand : public std::exception {
private:
    const char* _error_description;

public:
    explicit InvalidCommand(const char* errorDescription)
    : _error_description(errorDescription) {}

    const char* what() const noexcept override
    {
        return _error_description;
    }
};
#endif //EXCEPTION
