#include <iomanip>

#include "account.h"
#include "log.h"
#include "book.h"

LogGroup::LogGroup()
{
    std::ifstream tester("log");
    if (tester.good()) { // such file exists
        tester.close();
        _logs.open("log");
    } else { // no such file
        tester.close();

        // create file
        std::ofstream fileCreator("log");
        fileCreator.close();

        // open the file
        _logs.open("log");
    }

    tester.open("finance_log");
    if (tester.good()) { // such file exists
        tester.close();
        _finance_logs.open("finance_log");
    } else { // no such file
        tester.close();

        // create file
        std::ofstream fileCreator("finance_log");
        fileCreator.close();

        // open the file
        _finance_logs.open("finance_log");
    }
}

void LogGroup::addFinanceLog(FinanceLog& newLog)
{
    _finance_logs.seekp(0, std::ios::end);
    _finance_logs.write(reinterpret_cast<const char*>(&newLog), sizeof(FinanceLog));
}

void LogGroup::show(TokenScanner& line, const LoggingSituation& loggingStatus)
{
    if (loggingStatus.getPriority() < 7) throw InvalidCommand("Invalid");

    // skip the word "finance"
    line.nextToken();

    double income = 0;
    double expenditure = 0;

    if (line.hasMoreToken()) {
        string_t limitString = line.nextToken();
        if (line.hasMoreToken()) throw InvalidCommand("Invalid");
        int limit = stringToInt(limitString);
        if (limit == 0) {
            std::cout << std::endl;
            return;
        }
        _finance_logs.seekp(0, std::ios::end);
        const int end = _finance_logs.tellp();
        if (end / sizeof(FinanceLog) < limit) throw InvalidCommand("Invalid");
        FinanceLog financeLog;
        for (int i = end - limit * sizeof(FinanceLog); i < end; i += sizeof(FinanceLog)) {
            _finance_logs.seekg(i);
            _finance_logs.read(reinterpret_cast<char*>(&financeLog), sizeof(FinanceLog));
            if (financeLog.flag) income += financeLog.sum;
            else expenditure += financeLog.sum;
        }
    } else {
        _finance_logs.seekp(0, std::ios::end);
        const int end = _finance_logs.tellp();
        FinanceLog financeLog;
        std::cout << std::fixed << std::setprecision(2);
        for (int i = 0; i < end; i += sizeof(FinanceLog)) {
            _finance_logs.seekg(i);
            _finance_logs.read(reinterpret_cast<char*>(&financeLog), sizeof(FinanceLog));
            if (financeLog.flag) income += financeLog.sum;
            else expenditure += financeLog.sum;
        }
    }
    std::cout << std::fixed << std::setprecision(2) << "+ " << income << " - " << expenditure << std::endl;
}

void LogGroup::flush()
{
    _finance_logs.flush();
    _logs.flush();
}

void LogGroup::report(TokenScanner& line, const LoggingSituation& loggingStatus,
                      BookGroup& bookGroup, AccountGroup& accounts)
{
    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");

    string_t mode = line.nextToken();
    if (line.hasMoreToken()) throw InvalidCommand("Invalid");

    if (mode == "myself") {
        if (loggingStatus.getPriority() < 3) throw InvalidCommand("Invalid");
        UserID myself(loggingStatus.getID());
        _logs.seekp(0, std::ios::end);
        const int end = _logs.tellp();
        _logs.seekg(0);
        Log tmpLog;
        for (int i = 0; i < end; i += sizeof(Log)) {
            _logs.read(reinterpret_cast<char*>(&tmpLog), sizeof(Log));
            if (tmpLog.userID == myself) {
                if (tmpLog.behaviour == Log::buy) {
                    std::cout << "You bought  : " << tmpLog.quantity << " ";
                    if (bookGroup.find(tmpLog.offset).name.name[0] == '\0') {
                        std::cout << "< blank name >";
                    } else {
                        std::cout << bookGroup.find(tmpLog.offset).name.name;
                    }
                    std::cout << " (ISBN=" << bookGroup.find(tmpLog.offset).isbn.isbn
                              << ") with $" << std::fixed << std::setprecision(2)
                              << tmpLog.sum << std::endl;
                } else if (tmpLog.behaviour == Log::modify) {
                    std::cout << "You modified: ";
                    if (bookGroup.find(tmpLog.offset).name.name[0] == '\0') {
                        std::cout << "< blank name >";
                    } else {
                        std::cout << bookGroup.find(tmpLog.offset).name.name;
                    }
                    std::cout << " (ISBN=" << bookGroup.find(tmpLog.offset).isbn.isbn
                              << ") " << tmpLog.description << std::endl;
                } else if (tmpLog.behaviour == Log::import) {
                    std::cout << "You imported: " << tmpLog.quantity << " ";
                    if (bookGroup.find(tmpLog.offset).name.name[0] == '\0') {
                        std::cout << "< blank name >";
                    } else {
                        std::cout << bookGroup.find(tmpLog.offset).name.name;
                    }
                    std::cout << " (ISBN=" << bookGroup.find(tmpLog.offset).isbn.isbn
                              << ") with $" << std::fixed << std::setprecision(2)
                              << tmpLog.sum << std::endl;
                } else if (tmpLog.behaviour == Log::create) {
                    std::cout << "You created : ";
                    if (bookGroup.find(tmpLog.offset).name.name[0] == '\0') {
                        std::cout << "< blank name >";
                    } else {
                        std::cout << bookGroup.find(tmpLog.offset).name.name;
                    }
                    std::cout << " (ISBN=" << bookGroup.find(tmpLog.offset).isbn.isbn
                              << ")" << std::endl;
                } else if (tmpLog.behaviour == Log::login) {
                    std::cout << "You login" << std::endl;
                } else if (tmpLog.behaviour == Log::logout) {
                    std::cout << "You logout" << std::endl;
                } else if (tmpLog.behaviour == Log::changePassword) {
                    std::cout << "You changed " << tmpLog.description << std::endl;
                } else if (tmpLog.behaviour == Log::addUser) {
                    std::cout << "You added   : user " << tmpLog.description << std::endl;
                } else if (tmpLog.behaviour == Log::deleteUser) {
                    std::cout << "You deleted : " << tmpLog.description << std::endl;
                }
            }
        }
    } else if (mode == "finance") {
        if (loggingStatus.getPriority() < 7) throw InvalidCommand("Invalid");
        _reportFinance(bookGroup);
    } else if (mode == "employee") {
        if (loggingStatus.getPriority() < 7) throw InvalidCommand("Invalid");
        _reportEmployee(accounts, bookGroup);
    } else {
        throw InvalidCommand("Invalid");
    }
}

void LogGroup::addLog(Log& newLog)
{
    _logs.seekp(0, std::ios::end);
    _logs.write(reinterpret_cast<const char*>(&newLog), sizeof(Log));
}

void LogGroup::showLog(TokenScanner& line, const LoggingSituation& loggingStatus, BookGroup& bookGroup)
{
    if (loggingStatus.getPriority() < 7 || line.hasMoreToken()) throw InvalidCommand("Invalid");
    _logs.seekp(0, std::ios::end);
    const int end = _logs.tellp();
    _logs.seekg(0);
    Log tmpLog;
    for (int i = 0; i < end; i += sizeof(Log)) {
        _logs.read(reinterpret_cast<char*>(&tmpLog), sizeof(Log));
        if (tmpLog.behaviour == Log::buy) {
            std::cout << "[" << tmpLog.userID.ID << "]\tbought  : "
                      << tmpLog.quantity << " ";
            if (bookGroup.find(tmpLog.offset).name.name[0] == '\0') {
                std::cout << "< blank name >";
            } else {
                std::cout << bookGroup.find(tmpLog.offset).name.name;
            }
            std::cout << " (ISBN=" << bookGroup.find(tmpLog.offset).isbn.isbn
                      << ") with $" << std::fixed << std::setprecision(2)
                      << tmpLog.sum << std::endl;

        } else if (tmpLog.behaviour == Log::modify) {
            std::cout << "[" << tmpLog.userID.ID << "]\tmodified: ";
            if (bookGroup.find(tmpLog.offset).name.name[0] == '\0') {
                std::cout << "< blank name >";
            } else {
                std::cout << bookGroup.find(tmpLog.offset).name.name;
            }
            std::cout << " (ISBN=" << bookGroup.find(tmpLog.offset).isbn.isbn
                      << ") " << tmpLog.description << std::endl;

        } else if (tmpLog.behaviour == Log::import) {
            std::cout << "[" << tmpLog.userID.ID << "]\timported: "
                      << tmpLog.quantity << " ";
            if (bookGroup.find(tmpLog.offset).name.name[0] == '\0') {
                std::cout << "< blank name >";
            } else {
                std::cout << bookGroup.find(tmpLog.offset).name.name;
            }
            std::cout << " (ISBN=" << bookGroup.find(tmpLog.offset).isbn.isbn
                      << ") with $" << std::fixed << std::setprecision(2)
                      << tmpLog.sum << std::endl;

        } else if (tmpLog.behaviour == Log::create) {
            std::cout << "[" << tmpLog.userID.ID << "]\tcreated : ";
            if (bookGroup.find(tmpLog.offset).name.name[0] == '\0') {
                std::cout << "< blank name >";
            } else {
                std::cout << bookGroup.find(tmpLog.offset).name.name;
            }
            std::cout << " (ISBN=" << bookGroup.find(tmpLog.offset).isbn.isbn
                      << ")" << std::endl;
        } else if (tmpLog.behaviour == Log::login) {
            std::cout << "[" << tmpLog.userID.ID << "]\tlogin" << std::endl;
        } else if (tmpLog.behaviour == Log::logout) {
            std::cout << "[" << tmpLog.userID.ID << "]\tlogout" << std::endl;
        } else if (tmpLog.behaviour == Log::changePassword) {
            std::cout << "[" << tmpLog.userID.ID << "]\tchanged " << tmpLog.description << std::endl;
        } else if (tmpLog.behaviour == Log::addUser) {
            std::cout << "[" << tmpLog.userID.ID << "]\tadded   : user " << tmpLog.description << std::endl;
        } else if (tmpLog.behaviour == Log::deleteUser) {
            std::cout << "[" << tmpLog.userID.ID << "]\tdeleted : " << tmpLog.description << std::endl;
        }
    }
}

void LogGroup::_reportFinance(BookGroup& bookGroup)
{
    _logs.seekp(0, std::ios::end);
    const int end = _logs.tellp();
    _logs.seekg(0);
    Log tmpLog;
    for (int i = 0; i < end; i += sizeof(Log)) {
        _logs.read(reinterpret_cast<char*>(&tmpLog), sizeof(Log));
        if (tmpLog.behaviour == Log::buy) {
            std::cout << "+" << std::fixed << std::setprecision(2) << tmpLog.sum << "\t(["
                      << tmpLog.userID.ID << "] bought  : " << tmpLog.quantity << " ";
            if (bookGroup.find(tmpLog.offset).name.name[0] == '\0') {
                std::cout << "< blank name >";
            } else {
                std::cout << bookGroup.find(tmpLog.offset).name.name;
            }
            std::cout << " (ISBN=" << bookGroup.find(tmpLog.offset).isbn.isbn
                      << "))" << std::endl;
        } else if (tmpLog.behaviour == Log::import) {
            std::cout << "-" << std::fixed << std::setprecision(2) << tmpLog.sum << "\t(["
                      << tmpLog.userID.ID << "] imported: " << tmpLog.quantity << " ";
            if (bookGroup.find(tmpLog.offset).name.name[0] == '\0') {
                std::cout << " < blank name >";
            } else {
                std::cout << bookGroup.find(tmpLog.offset).name.name;
            }
            std::cout << " (ISBN=" << bookGroup.find(tmpLog.offset).isbn.isbn
                      << "))" << std::endl;
        }
    }
}

void LogGroup::_reportEmployee(AccountGroup& accounts, BookGroup& bookGroup)
{
    _logs.seekp(0, std::ios::end);
    const int end = _logs.tellp();
    _logs.seekg(0);
    Log tmpLog;
    for (int i = 0; i < end; i += sizeof(Log)) {
        _logs.read(reinterpret_cast<char*>(&tmpLog), sizeof(Log));
        if (tmpLog.priority == 3) {
            if (tmpLog.behaviour == Log::buy) {
                std::cout << "[" << tmpLog.userID.ID << "]\tbought  : "
                          << tmpLog.quantity << " ";
                if (bookGroup.find(tmpLog.offset).name.name[0] == '\0') {
                    std::cout << "< blank name >";
                } else {
                    std::cout << bookGroup.find(tmpLog.offset).name.name;
                }
                std::cout << " (ISBN=" << bookGroup.find(tmpLog.offset).isbn.isbn
                          << ") with $" << std::fixed << std::setprecision(2)
                          << tmpLog.sum << std::endl;
            } else if (tmpLog.behaviour == Log::modify) {
                std::cout << "[" << tmpLog.userID.ID << "]\tmodified: ";
                if (bookGroup.find(tmpLog.offset).name.name[0] == '\0') {
                    std::cout << "< blank name >";
                } else {
                    std::cout << bookGroup.find(tmpLog.offset).name.name;
                }
                std::cout << " (ISBN=" << bookGroup.find(tmpLog.offset).isbn.isbn
                          << ") " << tmpLog.description << std::endl;
            } else if (tmpLog.behaviour == Log::import) {
                std::cout << "[" << tmpLog.userID.ID << "]\timported: "
                          << tmpLog.quantity << " ";
                if (bookGroup.find(tmpLog.offset).name.name[0] == '\0') {
                    std::cout << "< blank name >";
                } else {
                    std::cout << bookGroup.find(tmpLog.offset).name.name;
                }
                std::cout << " (ISBN=" << bookGroup.find(tmpLog.offset).isbn.isbn
                          << ") with $" << std::fixed << std::setprecision(2)
                          << tmpLog.sum << std::endl;
            } else if (tmpLog.behaviour == Log::create) {
                std::cout << "[" << tmpLog.userID.ID << "]\tcreated : ";
                if (bookGroup.find(tmpLog.offset).name.name[0] == '\0') {
                    std::cout << "< blank name >";
                } else {
                    std::cout << bookGroup.find(tmpLog.offset).name.name;
                }
                std::cout << " (ISBN=" << bookGroup.find(tmpLog.offset).isbn.isbn
                          << ")" << std::endl;
            } else if (tmpLog.behaviour == Log::login) {
                std::cout << "[" << tmpLog.userID.ID << "]\tlogin" << std::endl;
            } else if (tmpLog.behaviour == Log::logout) {
                std::cout << "[" << tmpLog.userID.ID << "]\tlogout" << std::endl;
            } else if (tmpLog.behaviour == Log::changePassword) {
                std::cout << "[" << tmpLog.userID.ID << "]\tchanged " << tmpLog.description << std::endl;
            } else if (tmpLog.behaviour == Log::addUser) {
                std::cout << "[" << tmpLog.userID.ID << "]\tadded   : user " << tmpLog.description << std::endl;
            } else if (tmpLog.behaviour == Log::deleteUser) {
                std::cout << "[" << tmpLog.userID.ID << "]\tdeleted : " << tmpLog.description << std::endl;
            }
        }
    }
}

Log::Log(Behaviour behaviourIn, double sumIn, int quantityIn, bool flagIn, const UserID& userIDIn,
         int offsetIn, const string_t& descriptionIn, int priorityIn)
         : behaviour(behaviourIn), sum(sumIn), quantity(quantityIn), flag(flagIn),
           userID(userIDIn), offset(offsetIn), priority(priorityIn)
{
    for (int i = 0; i < descriptionIn.length(); ++i) {
        description[i] = descriptionIn[i];
    }
    description[descriptionIn.size()] = '\0';
}
