#ifndef LOG
#define LOG

#include <iostream>
#include <fstream>

class BookGroup;

struct FinanceLog {
    double sum;

    bool flag; // true to be income and false to be expenditure
};

struct Log {
    enum Behaviour {buy, create, modify, import, login, logout, changePassword, addUser, deleteUser};

    Behaviour behaviour;

    double sum;

    int quantity;

    bool flag; // true to be income and false to be expenditure

    UserID userID;

    int offset;

    char description[200];

    int priority;

    Log() = default;

    Log(Behaviour behaviourIn, double sumIn, int quantityIn, bool flagIn, const UserID& userIDIn,
        int offsetIn, const string_t& descriptionIn, int priorityIn);
};

class LogGroup {
private:
    std::fstream _logs;

    std::fstream _finance_logs;

    void _reportFinance(BookGroup& bookGroup);

    void _reportEmployee(AccountGroup& accounts, BookGroup& bookGroup);

public:
    LogGroup();

    ~LogGroup() = default;

    /**
     * COMMAND: report myself
     * <br>
     * COMMAND: report finance
     * <br>
     * COMMAND: report employee
     * @param line
     * @param loggingStatus
     */
    void report(TokenScanner& line, const LoggingSituation& loggingStatus,
                BookGroup& bookGroup, AccountGroup& accounts);

    void addLog(Log& newLog);

    void addFinanceLog(FinanceLog& newLog);

    /**
     * this function
     * @param line
     * @param loggingStatus
     */
    void show(TokenScanner& line, const LoggingSituation& loggingStatus);

    void showLog(TokenScanner& line, const LoggingSituation& loggingStatus, BookGroup& bookGroup);

    void flush();
};

#endif //LOG
