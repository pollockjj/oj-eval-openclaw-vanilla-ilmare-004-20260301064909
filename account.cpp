#include "account.h"
#include "log.h"

UserID::UserID()
{
    ID[0] = '\0';
}

UserID::UserID(const string_t& IDIn)
{
    for (int i = 0; i < IDIn.length(); ++i) {
        ID[i] = IDIn[i];
    }
    ID[IDIn.size()] = '\0';
}

bool UserID::operator==(const UserID& rhs) const
{
    for (int i = 0; i < 31; ++i) {
        if (this->ID[i] == '\0' && rhs.ID[i] == '\0') return true;
        if (this->ID[i] != rhs.ID[i]) return false;
    }
    return true;
}

bool UserID::operator<(const UserID& rhs) const
{
    for (int i = 0; i < 31; ++i) {
        if (this->ID[i] == '\0' && rhs.ID[i] == '\0') return false;
        if (this->ID[i] != rhs.ID[i]) return (this->ID[i] < rhs.ID[i]);
    }
    return false;
}

Account::Account() : ID(), priority(1)
{
    password[0] = '\0';
    name[0] = '\0';
}

Account::Account(const string_t& IDIn, const string_t& passwordIn,
                 const string_t& nameIn, int priorityIn) : ID(IDIn), priority(priorityIn)
{
    for (int i = 0; i < passwordIn.length(); ++i) {
        password[i] = passwordIn[i];
    }
    password[passwordIn.length()] = '\0';
    for (int i = 0; i < nameIn.length(); ++i) {
        name[i] = nameIn[i];
    }
    name[nameIn.size()] = '\0';
}

void Account::changePassword(const string_t& newPassword)
{
    for (int i = 0; i < newPassword.length(); ++i) {
        password[i] = newPassword[i];
    }
    password[newPassword.length()] = '\0';
}

void LoggingSituation::logIn(string_t logID, int priority, int bookOffset)
{
    ++_logged_num;
    _logged_in_ID.emplace_back(std::move(logID));
    _logged_in_priority.push_back(priority);
    _selected_book_offset.push_back(bookOffset);
}

void LoggingSituation::select(int bookOffset)
{
    _selected_book_offset.back() = bookOffset;
}

void LoggingSituation::logOut(LogGroup& logs)
{
    if (_logged_in_ID.empty()) throw InvalidCommand("Invalid");
    Log newLog(Log::logout, 0, 0, false, UserID(_logged_in_ID.back()),
               -1, string_t(), _logged_in_priority.back());
    logs.addLog(newLog);
    --_logged_num;
    _logged_in_ID.pop_back();
    _logged_in_priority.pop_back();
    _selected_book_offset.pop_back();
}

bool LoggingSituation::logged(const string_t& ID) const
{
    for (const string_t& loggedInID : _logged_in_ID) {
        if (ID == loggedInID) return true;
    }
    return false;
}

bool LoggingSituation::empty() const
{
    return (_logged_num == 0);
}

const string_t& LoggingSituation::getID() const
{
    return _logged_in_ID.back();
}

int LoggingSituation::getPriority() const
{
    if (empty()) return 0;
    else return _logged_in_priority.back();
}

int LoggingSituation::getSelected() const
{
    return _selected_book_offset.back();
}

AccountGroup::AccountGroup()
{
    std::ifstream tester("account");
    if (tester.good()) { // such file exists
        tester.close();
        _accounts.open("account");
    } else { // no such file
        tester.close();

        // create file "account"
        std::ofstream fileCreator("account");
        fileCreator.close();

        // add default root user
        _accounts.open("account");
        Account account("root", "sjtu", "root", 7);
        _add_user(account);
    }
}

void AccountGroup::_add_user(const Account& account)
{
    _accounts.seekp(0, std::ios::end);
    _id_index.insert(account.ID, _accounts.tellp());
    _accounts.write(reinterpret_cast<const char*>(&account), sizeof(account));
}

void AccountGroup::switchUser(TokenScanner& line, LoggingSituation& logStatus, LogGroup& logs)
{
    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");
    string_t userID = line.nextToken();
    if (!exist(userID)) throw InvalidCommand("Invalid");
    Account account = find(userID);

    if (!line.hasMoreToken()) {
        if (account.priority < logStatus.getPriority()) {
            logStatus.logIn(userID, account.priority, -1);

            Log newLog(Log::login, 0, 0, false, UserID(userID),
                       -1, string_t(), account.priority);
            logs.addLog(newLog);
        } else {
            throw InvalidCommand("Invalid");
        }
    } else {
        string_t password = line.nextToken();
        if (line.hasMoreToken()) throw InvalidCommand("Invalid");
        if (checkPassword(password, account)) {
            logStatus.logIn(userID, account.priority, -1);

            Log newLog(Log::login, 0, 0, false, UserID(userID),
                       -1, string_t(), account.priority);
            logs.addLog(newLog);
        } else {
            throw InvalidCommand("Invalid");
        }
    }
}

void AccountGroup::registerUser(TokenScanner& line)
{
    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");

    string_t userID = line.nextToken();
    if (!validUserID(userID)) throw InvalidCommand("Invalid");
    if (exist(userID)) throw InvalidCommand("Invalid");

    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");
    string_t password = line.nextToken();
    if (!validPassword(password)) throw InvalidCommand("Invalid");

    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");
    string_t userName = line.nextToken();
    if (!validUserName(userName)) throw InvalidCommand("Invalid");
    if (line.hasMoreToken()) throw InvalidCommand("Invalid");

    Account newAccount(userID, password, userName, 1);
    _add_user(newAccount);
}

void AccountGroup::addUser(TokenScanner& line, const LoggingSituation& logStatus, LogGroup& logs)
{
    // check authority
    if (logStatus.getPriority() < 3) throw InvalidCommand("Invalid");

    // check user ID
    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");
    string_t userID = line.nextToken();
    if (!validUserID(userID)) throw InvalidCommand("Invalid");
    if (exist(userID)) throw InvalidCommand("Invalid");

    // check password
    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");
    string_t password = line.nextToken();
    if (!validPassword(password)) throw InvalidCommand("Invalid");

    // check priority
    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");
    string_t priorityString = line.nextToken();
    int priority = stringToInt(priorityString);
    if (priority >= logStatus.getPriority()) throw InvalidCommand("Invalid");
    if (!validPriority(priority)) throw InvalidCommand("Invalid");

    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");
    string_t userName = line.nextToken();
    if (!validUserName(userName)) throw InvalidCommand("Invalid");
    if (line.hasMoreToken()) throw InvalidCommand("Invalid");

    Account newAccount(userID, password, userName, priority);
    _add_user(newAccount);

    string_t logDescription("[");
    logDescription += userID;
    logDescription += "] (name: ";
    logDescription += userName;
    logDescription += ", priority: ";
    logDescription += priorityString;
    logDescription += ')';
    Log newLog(Log::addUser, 0, 0, false, UserID(logStatus.getID()),
               -1, logDescription, logStatus.getPriority());
    logs.addLog(newLog);
}

void AccountGroup::deleteUser(TokenScanner& line, const LoggingSituation& logStatus, LogGroup& logs)
{
    // check authority
    if (logStatus.getPriority() < 7) throw InvalidCommand("Invalid");

    // check user ID
    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");
    string_t userID = line.nextToken();
    if (line.hasMoreToken() || !validUserID(userID)) throw InvalidCommand("Invalid");

    // check whether this user exists
    // (to reduce the time of finding the same account,
    // an inline code instead of calling exist() is necessary.)
    UserID ID(userID);
    int* position = _id_index.get(ID);
    if (position == nullptr) throw InvalidCommand("Invalid");
    delete position;

    // check whether this user has logged in
    if (logStatus.logged(userID)) throw InvalidCommand("Invalid");

    // delete user
    _id_index.erase(ID);

    string_t logDescription("[");
    logDescription += userID;
    logDescription += "]";
    Log newLog(Log::deleteUser, 0, 0, false, UserID(logStatus.getID()),
               -1, logDescription, logStatus.getPriority());
    logs.addLog(newLog);
}

Account AccountGroup::find(const string_t& userID)
{
    UserID ID(userID);
    int* position = _id_index.get(ID);
    if (position == nullptr) throw InvalidCommand("Invalid");
    _accounts.seekg(*position);
    delete position;
    Account account;
    _accounts.read(reinterpret_cast<char*>(&account), sizeof(Account));
    return account;
}

Account AccountGroup::find(const UserID& userID)
{
    int* position = _id_index.get(userID);
    if (position == nullptr) throw InvalidCommand("Invalid");
    _accounts.seekg(*position);
    delete position;
    Account account;
    _accounts.read(reinterpret_cast<char*>(&account), sizeof(Account));
    return account;
}

bool AccountGroup::exist(const string_t& userID)
{
    UserID ID(userID);
    int* position = _id_index.get(ID);
    if (position == nullptr) return false;
    delete position;
    return true;
}

void AccountGroup::changePassword(TokenScanner& line, const LoggingSituation& logStatus, LogGroup& logs)
{
    // check authority
    if (logStatus.getPriority() < 1) throw InvalidCommand("Invalid");

    // check user ID
    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");
    string_t userID = line.nextToken();
    if (!validUserID(userID)) throw InvalidCommand("Invalid");

    // check whether this user exists
    // (to reduce the time of finding the same account,
    // an inline code instead of calling exist() is necessary.)
    UserID ID(userID);
    int* position = _id_index.get(ID);
    if (position == nullptr) throw InvalidCommand("Invalid");

    // check the first password
    if (!line.hasMoreToken()) {
        delete position;
        position = nullptr;
        throw InvalidCommand("Invalid");
    }
    string_t password1 = line.nextToken();
    if (!validPassword(password1)) {
        delete position;
        position = nullptr;
        throw InvalidCommand("Invalid");
    }

    if (!line.hasMoreToken()) {
        if (logStatus.getPriority() == 7) {
            Account account;
            _accounts.seekg(*position);
            _accounts.read(reinterpret_cast<char*>(&account), sizeof(Account));
            account.changePassword(password1);
            _accounts.seekp(*position);
            _accounts.write(reinterpret_cast<char*>(&account), sizeof(Account));

            string_t logDescription;
            if (userID == logStatus.getID()) {
                logDescription = "his / her own password";
            } else {
                logDescription = "[";
                logDescription += userID;
                logDescription += "]\'s password";
            }
            Log newLog(Log::changePassword, 0, 0, false, UserID(logStatus.getID()),
                       -1, logDescription, logStatus.getPriority());
            logs.addLog(newLog);
        } else {
            delete position;
            position = nullptr;
            throw InvalidCommand("Invalid");
        }
    } else {
        Account account;
        _accounts.seekg(*position);
        _accounts.read(reinterpret_cast<char*>(&account), sizeof(Account));

        //check the accuracy of old password
        for (int i = 0; i < password1.length(); ++i) {
            if (password1[i] != account.password[i]) {
                delete position;
                position = nullptr;
                throw InvalidCommand("Invalid");
            }
        }
        if (account.password[password1.length()] != '\0') {
            delete position;
            position = nullptr;
            throw InvalidCommand("Invalid");
        }

        // check the second password
        string_t password2 = line.nextToken();
        if (line.hasMoreToken()) {
            delete position;
            position = nullptr;
            throw InvalidCommand("Invalid");
        }
        if (!validPassword(password2)) {
            delete position;
            position = nullptr;
            throw InvalidCommand("Invalid");
        }
        account.changePassword(password2);
        _accounts.seekp(*position);
        _accounts.write(reinterpret_cast<char*>(&account), sizeof(Account));

        string_t logDescription;
        if (userID == logStatus.getID()) {
            logDescription = "his / her own password";
        } else {
            logDescription = "[";
            logDescription += userID;
            logDescription += "]\'s password";
        }
        Log newLog(Log::changePassword, 0, 0, false, UserID(logStatus.getID()),
                   -1, logDescription, logStatus.getPriority());
        logs.addLog(newLog);
    }

    delete position;
}

void AccountGroup::flush()
{
    _accounts.flush();
    _id_index.flush();
}

bool checkPassword(const string_t& input, const Account& account) {
    for (int i = 0; i < input.length(); ++i) {
        if (input[i] != account.password[i]) {
            return false;
        }
    }
    if (account.password[input.length()] != '\0') return false;
    else return true;
}

bool validUserID(const string_t& userID)
{
    if (userID.empty() || userID.length() > 30) return false;
    for (char_t c : userID) {
        if ((c < 48) || (c > 57 && c < 65)
         || (c > 90 && c < 95) || (c == 96) || (c > 122)) {
            return false;
        }
    }
    return true;
}

bool validPassword(const string_t& password)
{
    if (password.empty() || password.length() > 30) return false;
    for (char_t c : password) {
        if ((c < 48) || (c > 57 && c < 65)
            || (c > 90 && c < 95) || (c == 96) || (c > 122)) {
            return false;
        }
    }
    return true;
}

bool validUserName(const string_t& userName)
{
    if (userName.empty() || userName.length() > 30) return false;
    for (char_t c : userName) {
        if (c < 33 || c > 126) return false;
    }
    return true;
}

bool validPriority(int priority) {
    if ((priority != 7) && (priority != 3) && (priority != 1)) return false;
    return true;
}