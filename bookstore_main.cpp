// Authored by Lau Yee-Yu
// Using ISO CPP standard

#include <iostream>

#include "exception.h"
#include "token_scanner.h"
#include "account.h"
#include "book.h"
#include "log.h"

bool processLine(AccountGroup& accounts, BookGroup& books,
                 LogGroup& logs, LoggingSituation& logInStack);

void init();

int main()
{
    init();
    AccountGroup accounts;
    BookGroup books;
    LogGroup logs;
    LoggingSituation logInStack;
    while (true) {
        try {
            if (processLine(accounts, books, logs, logInStack)) return 0;
        } catch (std::exception& ex) {
            std::cout << ex.what() << std::endl;
        }
    }
}

bool processLine(AccountGroup& accounts, BookGroup& books,
                 LogGroup& logs, LoggingSituation& logInStack)
{
    TokenScanner line;
    line.newLine();

    if (!std::cin) return true;

    if (line.totalLength() > 1024) throw InvalidCommand("Invalid");

    if (!line.hasMoreToken()) return false;

    string_t command = line.nextToken();
    if (command == "quit" || command == "exit") {
        if (line.hasMoreToken()) throw InvalidCommand("Invalid");
        return true;
    } else if (command == "su") {
        accounts.switchUser(line, logInStack,logs);
    } else if (command == "logout") {
        if (line.hasMoreToken()) throw InvalidCommand("Invalid");
        logInStack.logOut(logs);
    } else if (command == "register") {
        accounts.registerUser(line);
    } else if (command == "passwd") {
        accounts.changePassword(line, logInStack, logs);
    } else if (command == "useradd") {
        accounts.addUser(line, logInStack, logs);
    } else if (command == "delete") {
        accounts.deleteUser(line, logInStack, logs);
    } else if (command == "show") {
        books.show(line, logInStack, logs);
    } else if (command == "buy") {
        books.buy(line, logInStack, logs);
    } else if (command == "select") {
        books.select(line, logInStack, logs);
    } else if (command == "modify") {
        books.modify(line, logInStack, logs);
    } else if (command == "import") {
        books.importBook(line, logInStack, logs);
    } else if (command == "report") {
        logs.report(line, logInStack, books, accounts);
    } else if (command == "log") {
        logs.showLog(line, logInStack, books);
    } else {
        throw InvalidCommand("Invalid");
    }
    return false;
}

void init()
{
    std::ifstream tester("account_index");
    if (!(tester.good())) {
        std::ofstream creator("account_index");
        creator.close();
    }
    tester.close();

    tester.open("book_index_ISBN");
    if (!(tester.good())) {
        std::ofstream creator("book_index_ISBN");
        creator.close();
    }
    tester.close();

    tester.open("book_index_author");
    if (!(tester.good())) {
        std::ofstream creator("book_index_author");
        creator.close();
    }
    tester.close();

    tester.open("book_index_name");
    if (!(tester.good())) {
        std::ofstream creator("book_index_name");
        creator.close();
    }
    tester.close();

    tester.open("book_index_keyword");
    if (!(tester.good())) {
        std::ofstream creator("book_index_keyword");
        creator.close();
    }
    tester.close();
}
