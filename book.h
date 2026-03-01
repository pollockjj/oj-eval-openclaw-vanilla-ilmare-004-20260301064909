#ifndef BOOK
#define BOOK

#include <iostream>
#include <fstream>

#include "unrolled_linked_list.h"
#include "token_scanner.h"

class LoggingSituation;
class LogGroup;

typedef char char_t;
typedef std::string string_t;

enum bookInformationType {isbn, name, author, keywords, price};

struct ISBN {
    char_t isbn[21];

    ISBN();

    explicit ISBN(const string_t& isbn_in);

    bool operator==(const ISBN& rhs) const;

    bool operator<(const ISBN& rhs) const;
};

struct Name {
    char_t name[61];

    Name();

    explicit Name(const string_t& name_in);

    bool operator==(const Name& rhs) const;

    bool operator<(const Name& rhs) const;
};

struct Author {
    char_t author[61];

    Author();

    explicit Author(const string_t& author_in);

    bool operator==(const Author& rhs) const;

    bool operator<(const Author& rhs) const;
};

struct Keyword {
    char_t keyword[61];

    Keyword();

    explicit Keyword(const string_t& keyword_in);

    bool operator==(const Keyword& rhs) const;

    bool operator<(const Keyword& rhs) const;
};

struct Keywords {
    char_t keywords[61];

    Keywords();

    explicit Keywords(const string_t& keywords_in);
};

struct Book {
    ISBN isbn;

    Name name;

    Author author;

    Keywords keywords;

    int quantity = 0;

    double price = 0;

    Book(const string_t& isbn_in, const string_t& name_in, const string_t& Author_in,
         const string_t& keywords_in, int quantity_in, double price_in);

    explicit Book(const string_t& isbn_in);

    Book() = default;

    friend std::ostream& operator<<(std::ostream& os, const Book& book);
};

class BookGroup {
private:
    UnrolledLinkedList<ISBN, int> _isbn_book_map
    = UnrolledLinkedList<ISBN, int>("book_index_ISBN");

    DoubleUnrolledLinkedList<Name, ISBN, int> _name_book_map
    = DoubleUnrolledLinkedList<Name, ISBN, int>("book_index_name");

    DoubleUnrolledLinkedList<Author, ISBN, int> _author_book_map
    = DoubleUnrolledLinkedList<Author, ISBN, int>("book_index_author");

    DoubleUnrolledLinkedList<Keyword, ISBN, int> _keywords_book_map
    = DoubleUnrolledLinkedList<Keyword, ISBN, int>("book_index_keyword");

    std::fstream _books;

    int all_book_num = 0;

public:
    BookGroup();

    ~BookGroup() = default;

    /**
     * This function search a book with ISBN.
     * @param offset
     * @return the book data
     */
    Book find(int offset);

    /**
     * This function check whether there is a logged-in user first.
     * Then the function read the next token and print books that
     * agrees with the request.  The case of an empty parameter
     * description or multiple parameters is regarded as invalid.
     * <br><br>
     * COMMAND: show (-ISBN=[ISBN] | -name="[Book-Name]" |
     * -author="[Author]" | -keyword="[Keyword]")?
     * @param line
     * @param loggingStatus
     * @param logGroup
     */
    void show(TokenScanner& line, const LoggingSituation& loggingStatus, LogGroup& logGroup);

    /**
     * This function modify the data of the selected book.  The case
     * that no book is selected, or the case of no parameters at all,
     * or the case of repeated parameters, or the case of repeated
     * keywords are all invalid.  By the way, to avoid problems about
     * the show finance keyword, the function must check whether is
     * command is show finance command at first.  Then, this function
     * will check the authority of user (need to be no less than 3).
     * <br><br>
     * COMMAND: modify (-ISBN=[ISBN] | -name="[Book-Name]" |
     * -author="[Author]" | -keyword="[Keyword]" | -price=[Price])+
     * @param line
     * @param loggingStatus
     * @param logGroup
     */
    void modify(TokenScanner& line, const LoggingSituation& loggingStatus, LogGroup& logGroup);

    /**
     * This function check whether there is a logged-in user first.
     * Then it reads the isbn and check whether a book exists or not.
     * Then it buy the book and print the book cost.
     * <br><br>
     * COMMAND: buy [ISBN] [Quantity]
     * @param line
     * @param loggingStatus
     * @param logGroup
     */
    void buy(TokenScanner& line, const LoggingSituation& loggingStatus, LogGroup& logGroup);

    /**
     * This function will check the authority of user first (need to
     * be no less than 3).  If no book is selected, then it will throw
     * an exception.
     * <br><br>
     * COMMAND: import [Quantity] [Total-Cost]
     * @param line
     * @param loggingStatus
     * @param logGroup
     */
    void importBook(TokenScanner& line, const LoggingSituation& loggingStatus, LogGroup& logGroup);

    /**
     * This function will check the authority of user first (need to
     * be no less than 3).  If no book is selected, then it will throw
     * an exception.
     * <br><br>
     * COMMAND: select [ISBN]
     * @param line
     * @param loggingStatus
     * @param logGroup
     */
    void select(TokenScanner& line, LoggingSituation& loggingStatus, LogGroup& logGroup);

    void flush();
};

bool validISBN(const string_t& ISBN);

bool validBookName(const string_t& name);

bool validAuthor(const string_t& author);

bool validKeyword(const string_t& keyword);

bool validKeywords(const string_t& keywords);

bool validPrice(const string_t& price);

struct BookParameter {
    bookInformationType type;

    string_t content;
};

/**
 * This function parses a token into a book parameter.  The validity
 * is also checked here.
 * @param token
 * @return a bookParameter struct
 */
BookParameter processParameter(const string_t& token);

#endif //BOOK
