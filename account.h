#ifndef ACCOUNT
#define ACCOUNT

#include <iostream>
#include <fstream>

#include "unrolled_linked_list.h"
#include "token_scanner.h"
#include "exception.h"

typedef char char_t;
typedef std::string string_t;

class LogGroup;

/**
 * @struct UserID
 *
 * This struct stores a user ID with overridden operators (== and \<).
 */
struct UserID {
    char_t ID[31];

    UserID();

    /**
     * The most used constructor o UserID.
     * <br>
     * WARNING: The IDIn CANNOT be longer than 30 characters.
     */
    explicit UserID(const string_t& IDIn);

    ~UserID() = default;

    bool operator==(const UserID& rhs) const;

    bool operator<(const UserID& rhs) const;
};

/**
 * @struct Account
 *
 * This struct store every detail of an account.
 */
struct Account {
    UserID ID;
    char_t password[31];
    char_t name[31];
    int priority;

    Account();

    Account(const string_t& IDIn, const string_t& passwordIn,
            const string_t& nameIn, int priorityIn);

    ~Account() = default;

    void changePassword(const string_t& newPassword);
};

/**
 * @class LoggingSituation
 *
 * This class is used to store the stack of the logged account(s).
 */
class LoggingSituation {
private:
    int _logged_num = 0; // the number of logged-in accounts

    std::vector<string_t> _logged_in_ID; // to store the UserID

    std::vector<int> _logged_in_priority; // to store the related priority

    std::vector<int> _selected_book_offset; // to store the selected book id

public:
    LoggingSituation() = default;

    ~LoggingSituation() = default;

    /**
     * This function is to add a logged-in account and some data about
     * this account.
     * <br><br>
     * WARNING: The correctness of the data should be checked before
     * calling this function.
     * @param logID
     * @param priority
     * @param bookID
     */
    void logIn(string_t logID, int priority, int bookOffset);

    /**
     * This function log out the account that is at the top of the
     * logging stack.
     */
    void logOut(LogGroup& logs);

    /**
     * This function change the selected book of the user at the top
     * of the logging stack.
     * @param bookOffset
     */
    void select(int bookOffset);

    /**
     * This function check whether such ID is logged in.
     * @param ID the ID to be checked whether it is logged in.
     * @return the boolean of whether such account is logged in.
     */
    [[nodiscard]] bool logged(const string_t& ID) const;

    /**
     * This function return whether it is an empty logging stack.
     */
    [[nodiscard]] bool empty() const;

    [[nodiscard]] const string_t& getID() const;

    [[nodiscard]] int getPriority() const;

    [[nodiscard]] int getSelected() const;
};

class AccountGroup {
private:
    UnrolledLinkedList<UserID, int> _id_index = UnrolledLinkedList<UserID, int>("account_index");

    std::fstream _accounts;

    /**
     * This function add a user in the database.
     * @param account The new account to be added to the database
     */
    void _add_user(const Account& account);

public:
    AccountGroup();

    ~AccountGroup() = default;

    /**
     * This function is used for su command.  If the user on the current
     * logging stack is prior to the user to log in, password can be
     * omitted (having an password is valid); otherwise, password is
     * a must.
     * <br><br>
     * COMMAND: su [User-ID] ([Password])?
     * @param line The line user typed
     * @param logStatus The logging stack
     */
    void switchUser(TokenScanner& line, LoggingSituation& logStatus, LogGroup& logs);

    /**
     * This function registers a user with priority 1.  NO authority
     * is required here.  If there is such userID, then the operation
     * is denied.
     * <br><br>
     * COMMAND: register [User-ID] [Password] [User-Name]
     * @param line
     * @param logStatus
     */
    void registerUser(TokenScanner& line);

    /**
     * This function adds a user.  Different form the registerUser,
     * This function can assign a user with higher priority.  However,
     * the priority MUST be smaller than the current one, and the userID
     * CANNOT be the same of existing ones.
     * <br><br>
     * COMMAND: useradd [User-ID] [Password] [Priority] [User-Name]
     * @param line
     * @param logStatus
     */
    void addUser(TokenScanner& line, const LoggingSituation& logStatus, LogGroup& logs);

    /**
     * This function deletes a user.  The priority MUST be 7.  The case of
     * logged-in account and not existing account will cause an exception.
     * <br><br>
     * COMMAND: delete [User-ID]
     * @param line
     * @param logStatus
     */
    void deleteUser(TokenScanner& line, const LoggingSituation& logStatus, LogGroup& logs);

    /**
     * This function returns a Account class.
     * @param userID
     * @return the related account
     */
    Account find(const string_t& userID);

    /**
     * This function returns a Account class.
     * @param userID
     * @return the related account
     */
    Account find(const UserID& userID);

    /**
     * This function tells you whether a userID exists.
     * @param userID
     * @return the boolean of whether a userID exists
     */
    bool exist(const string_t& userID);

    /**
     * This function changes the password of a certain user.  There
     * must be at least one logged-in user.  If no such account or
     * password is wrong, the function will throw an exception.  For
     * root user (priority 7), old password can be omitted.
     * <br><br>
     * COMMAND: passwd [User-ID] ([Old-Password])? [New-Password]
     * @param line
     * @param logStatus
     */
    void changePassword(TokenScanner& line, const LoggingSituation& logStatus, LogGroup& logs);

    void flush();
};

bool checkPassword(const string_t& input, const Account& account);

bool validUserID(const string_t& userID);

bool validPassword(const string_t& password);

bool validUserName(const string_t& userName);

bool validPriority(int priority);

#endif //ACCOUNT
