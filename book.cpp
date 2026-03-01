#include <iomanip>
#include <set>

#include "book.h"
#include "account.h"
#include "log.h"

ISBN::ISBN()
{
    isbn[0] = '\0';
}

ISBN::ISBN(const string_t& isbn_in)
{
    for (int i = 0; i < isbn_in.length(); ++i) {
        isbn[i] = isbn_in[i];
    }
    isbn[isbn_in.size()] = '\0';
}

bool ISBN::operator==(const ISBN& rhs) const
{
    for (int i = 0; i < 21; ++i) {
        if (this->isbn[i] == '\0' && rhs.isbn[i] == '\0') return true;
        if (this->isbn[i] != rhs.isbn[i]) return false;
    }
    return true;
}

bool ISBN::operator<(const ISBN& rhs) const
{
    for (int i = 0; i < 21; ++i) {
        if (this->isbn[i] == '\0' && rhs.isbn[i] == '\0') return false;
        if (this->isbn[i] != rhs.isbn[i]) return (this->isbn[i] < rhs.isbn[i]);
    }
    return false;
}

Name::Name()
{
    name[0] = '\0';
}

Name::Name(const string_t& name_in)
{
    for (int i = 0; i < name_in.length(); ++i) {
        name[i] = name_in[i];
    }
    name[name_in.size()] = '\0';
}

bool Name::operator==(const Name& rhs) const
{
    for (int i = 0; i < 61; ++i) {
        if (this->name[i] == '\0' && rhs.name[i] == '\0') return true;
        if (this->name[i] != rhs.name[i]) return false;
    }
    return true;
}

bool Name::operator<(const Name& rhs) const
{
    for (int i = 0; i < 61; ++i) {
        if (this->name[i] == '\0' && rhs.name[i] == '\0') return false;
        if (this->name[i] != rhs.name[i]) return (this->name[i] < rhs.name[i]);
    }
    return false;
}

Author::Author()
{
    author[0] = '\0';
}

Author::Author(const string_t& author_in)
{
    for (int i = 0; i < author_in.length(); ++i) {
        author[i] = author_in[i];
    }
    author[author_in.size()] = '\0';
}

bool Author::operator==(const Author& rhs) const
{
    for (int i = 0; i < 61; ++i) {
        if (this->author[i] == '\0' && rhs.author[i] == '\0') return true;
        if (this->author[i] != rhs.author[i]) return false;
    }
    return true;
}

bool Author::operator<(const Author& rhs) const
{
    for (int i = 0; i < 61; ++i) {
        if (this->author[i] == '\0' && rhs.author[i] == '\0') return false;
        if (this->author[i] != rhs.author[i]) return (this->author[i] < rhs.author[i]);
    }
    return false;
}

Keyword::Keyword()
{
    keyword[0] = '\0';
}

Keyword::Keyword(const string_t& keyword_in)
{
    for (int i = 0; i < keyword_in.length(); ++i) {
        keyword[i] = keyword_in[i];
    }
    keyword[keyword_in.size()] = '\0';
}

bool Keyword::operator==(const Keyword& rhs) const
{
    for (int i = 0; i < 61; ++i) {
        if (this->keyword[i] == '\0' && rhs.keyword[i] == '\0') return true;
        if (this->keyword[i] != rhs.keyword[i]) return false;
    }
    return true;
}

bool Keyword::operator<(const Keyword& rhs) const
{
    for (int i = 0; i < 61; ++i) {
        if (this->keyword[i] == '\0' && rhs.keyword[i] == '\0') return false;
        if (this->keyword[i] != rhs.keyword[i]) return (this->keyword[i] < rhs.keyword[i]);
    }
    return false;
}

Keywords::Keywords()
{
    keywords[0] = '\0';
}

Keywords::Keywords(const string_t& keywords_in)
{
    for (int i = 0; i < keywords_in.length(); ++i) {
        keywords[i] = keywords_in[i];
    }
    keywords[keywords_in.size()] = '\0';
}

Book::Book(const string_t& isbn_in, const string_t& name_in, const string_t& author_in,
           const string_t& keywords_in, int quantity_in, double price_in)
           : isbn(isbn_in), name(name_in), author(author_in), keywords(keywords_in),
             quantity(quantity_in) {}

Book::Book(const string_t& isbn_in) : isbn(isbn_in) {}

std::ostream& operator<<(std::ostream& os, const Book& book)
{
    os << book.isbn.isbn << "\t"
       << book.name.name << "\t"
       << book.author.author << "\t"
       << book.keywords.keywords << "\t"
       << std::fixed << std::setprecision(2) << book.price << "\t"
       << book.quantity;
    return os;
}

BookGroup::BookGroup()
{
    std::ifstream tester("book");
    if (tester.good()) { // such file exists
        tester.close();
        _books.open("book");
    } else { // no such file
        tester.close();

        // create file "book"
        std::ofstream fileCreator("book");
        fileCreator.close();

        // open the file with _book
        _books.open("book");
    }
}

Book BookGroup::find(int offset)
{
    _books.seekg(offset);
    Book book;
    _books.read(reinterpret_cast<char*>(&book), sizeof(Book));
    return book;
}

void BookGroup::show(TokenScanner& line, const LoggingSituation& loggingStatus, LogGroup& logGroup)
{
    // the case of finance
    if (line.peekNextToken() == "finance") {
        logGroup.show(line, loggingStatus);
        return;
    }

    if (loggingStatus.empty()) throw InvalidCommand("Invalid");

    std::vector<int> books;
    if (!line.hasMoreToken()) {
        books = _isbn_book_map.traverse();
    } else {
        string_t parameter = line.nextToken();
        if (line.hasMoreToken()) throw InvalidCommand("Invalid");

        BookParameter bookParameter = processParameter(parameter);

        if (bookParameter.type == isbn) {
            int* offset = _isbn_book_map.get(ISBN(bookParameter.content));

            if (offset == nullptr) {
                std::cout << std::endl;
                return;
            } else {
                books.push_back(*offset);
                delete offset;
            }

        } else if (bookParameter.type == name) {
            books = _name_book_map.traverse(Name(bookParameter.content));

        } else if (bookParameter.type == author) {
            books = _author_book_map.traverse(Author(bookParameter.content));

        } else if (bookParameter.type == keywords) {
            for (char_t c : bookParameter.content) {
                if (c == '|') throw InvalidCommand("Invalid");
            }
            books = _keywords_book_map.traverse(Keyword(bookParameter.content));
        } else {
            throw InvalidCommand("Invalid");
        }
    }

    if (books.empty()) {
        std::cout << std::endl;
        return;
    }

    Book tmp;
    for (int offset : books) {
        _books.seekg(offset);
        _books.read(reinterpret_cast<char*>(&tmp), sizeof(Book));
        std::cout << tmp << std::endl;
    }
}

void BookGroup::modify(TokenScanner& line, const LoggingSituation& loggingStatus, LogGroup& logGroup)
{
    if (loggingStatus.getPriority() < 3) throw InvalidCommand("Invalid");
    if (loggingStatus.getSelected() == -1) throw InvalidCommand("Invalid");
    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");

    bool existISBN = false;
    bool existName = false;
    bool existAuthor = false;
    bool existKeywords = false;
    bool existPrice = false;
    std::vector<BookParameter> toModify;
    string_t token;
    while (line.hasMoreToken()) {
        token = line.nextToken();
        toModify.emplace_back(processParameter(token));
        if (toModify.back().type == isbn) {
            if (existISBN) throw InvalidCommand("Invalid");
            else {
                int* offset = _isbn_book_map.get(ISBN(toModify.back().content));
                if (offset != nullptr) {
                    delete offset;
                    throw InvalidCommand("Invalid");
                }
                existISBN = true;
            }
        } else if (toModify.back().type == name) {
            if (existName) throw InvalidCommand("Invalid");
            else existName = true;
        } else if (toModify.back().type == author) {
            if (existAuthor) throw InvalidCommand("Invalid");
            else existAuthor = true;
        } else if (toModify.back().type == keywords) {
            if (existKeywords) throw InvalidCommand("Invalid");
            else existKeywords = true;
        } else if (toModify.back().type == price) {
            if (existPrice) throw InvalidCommand("Invalid");
            else existPrice = true;
        }
    }

    // get book first
    Book bookToModify;
    _books.seekg(loggingStatus.getSelected());
    _books.read(reinterpret_cast<char*>(&bookToModify), sizeof(Book));

    // modify the data
    for (const BookParameter& bookParameter: toModify) {
        if (bookParameter.type == isbn) {
            ISBN newISBN(bookParameter.content);

            // add a log
            string_t logDescription("ISBN ");
            logDescription += bookToModify.isbn.isbn;
            logDescription += " -> ";
            logDescription += bookParameter.content;
            Log newLog(Log::modify, 0, 0, false, UserID(loggingStatus.getID()),
                       loggingStatus.getSelected(), logDescription, loggingStatus.getPriority());
            logGroup.addLog(newLog);

            // ISBN
            _isbn_book_map.erase(bookToModify.isbn);
            _isbn_book_map.insert(newISBN, loggingStatus.getSelected());

            // book name
            if (bookToModify.name.name[0] != '\0') {
                _name_book_map.erase(bookToModify.name, bookToModify.isbn);
                _name_book_map.insert(bookToModify.name, newISBN, loggingStatus.getSelected());
            }

            // author
            if (bookToModify.author.author[0] != '\0') {
                _author_book_map.erase(bookToModify.author, bookToModify.isbn);
                _author_book_map.insert(bookToModify.author, newISBN, loggingStatus.getSelected());
            }

            // keywords
            if (bookToModify.keywords.keywords[0] != '\0') {
                TokenScanner keywordSeparator(string_t(bookToModify.keywords.keywords),
                                              '|', TokenScanner::single);
                Keyword keywordBuffer;
                while (keywordSeparator.hasMoreToken()) {
                    keywordBuffer = Keyword(keywordSeparator.nextToken());
                    _keywords_book_map.erase(keywordBuffer, bookToModify.isbn);
                    _keywords_book_map.insert(keywordBuffer, newISBN, loggingStatus.getSelected());
                }
            }

            bookToModify.isbn = newISBN;

        } else if (bookParameter.type == name) {
            Name newName(bookParameter.content);

            // add a log
            string_t logDescription("book name: ");
            if (bookToModify.name.name[0] != '\0') {
                logDescription += bookToModify.name.name;
            } else {
                logDescription += "< blank name >";
            }
            logDescription += " -> ";
            logDescription += bookParameter.content;
            Log newLog(Log::modify, 0, 0, false, UserID(loggingStatus.getID()),
                       loggingStatus.getSelected(), logDescription, loggingStatus.getPriority());
            logGroup.addLog(newLog);

            if (bookToModify.name.name[0] != '\0') {
                _name_book_map.erase(bookToModify.name, bookToModify.isbn);
            }
            _name_book_map.insert(newName, bookToModify.isbn, loggingStatus.getSelected());

            bookToModify.name = newName;

        } else if (bookParameter.type == author) {
            Author newAuthor(bookParameter.content);

            // add a log
            string_t logDescription("author: ");
            if (bookToModify.author.author[0] != '\0') {
                logDescription += bookToModify.author.author;
            } else {
                logDescription += "< blank author >";
            }
            logDescription += " -> ";
            logDescription += bookParameter.content;
            Log newLog(Log::modify, 0, 0, false, UserID(loggingStatus.getID()),
                       loggingStatus.getSelected(), logDescription, loggingStatus.getPriority());
            logGroup.addLog(newLog);

            if (bookToModify.author.author[0] != '\0') {
                _author_book_map.erase(bookToModify.author, bookToModify.isbn);
            }
            _author_book_map.insert(newAuthor, bookToModify.isbn, loggingStatus.getSelected());

            bookToModify.author = newAuthor;

        } else if (bookParameter.type == keywords) {
            // add a log
            string_t logDescription("keyword: ");
            if (bookToModify.keywords.keywords[0] != '\0') {
                logDescription += bookToModify.keywords.keywords;
            } else {
                logDescription += "< blank keyword >";
            }
            logDescription += " -> ";
            logDescription += bookParameter.content;
            Log newLog(Log::modify, 0, 0, false, UserID(loggingStatus.getID()),
                       loggingStatus.getSelected(), logDescription, loggingStatus.getPriority());
            logGroup.addLog(newLog);

            // clear the old keywords
            if (bookToModify.keywords.keywords[0] != '\0') {
                TokenScanner keywordSeparator(string_t(bookToModify.keywords.keywords),
                                              '|', TokenScanner::single);
                Keyword keywordBuffer;
                while (keywordSeparator.hasMoreToken()) {
                    keywordBuffer = Keyword(keywordSeparator.nextToken());
                    _keywords_book_map.erase(keywordBuffer, bookToModify.isbn);
                }
            }

            // add new keywords
            bookToModify.keywords = Keywords(bookParameter.content);
            TokenScanner newKeywordSeparator(bookParameter.content,
                                             '|', TokenScanner::single);
            while (newKeywordSeparator.hasMoreToken()) {
                _keywords_book_map.insert(Keyword(newKeywordSeparator.nextToken()),
                                          bookToModify.isbn, loggingStatus.getSelected());
            }

        } else if (bookParameter.type == price) {
            // add a log
            string_t logDescription("price: ");
            std::stringstream oldPriceStream;
            oldPriceStream << std::fixed << std::setprecision(2) << bookToModify.price;
            logDescription += oldPriceStream.str();
            logDescription += " -> ";
            double newPrice = stringToDouble(bookParameter.content);
            std::stringstream newPriceStream;
            newPriceStream << std::fixed << std::setprecision(2) << newPrice;
            logDescription += newPriceStream.str();
            Log newLog(Log::modify, 0, 0, false, UserID(loggingStatus.getID()),
                       loggingStatus.getSelected(), logDescription, loggingStatus.getPriority());
            logGroup.addLog(newLog);

            bookToModify.price = newPrice;
        }

        // Put the book back
        _books.seekp(loggingStatus.getSelected());
        _books.write(reinterpret_cast<const char*>(&bookToModify), sizeof(Book));
    }
}

void BookGroup::buy(TokenScanner& line, const LoggingSituation& loggingStatus, LogGroup& logGroup)
{
    if (loggingStatus.empty()) throw InvalidCommand("Invalid");

    // read and check the isbn
    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");
    string_t ISBNString = line.nextToken();
    if (!validISBN(ISBNString)) throw InvalidCommand("Invalid");
    int* offset = _isbn_book_map.get(ISBN(ISBNString));
    if (offset == nullptr) throw InvalidCommand("Invalid");

    // read the quantity
    if (!line.hasMoreToken()) {
        delete offset;
        throw InvalidCommand("Invalid");
    }
    string_t quantityString = line.nextToken();
    if (line.hasMoreToken()) {
        delete offset;
        throw InvalidCommand("Invalid");
    }
    for (char_t c : quantityString) {
        if (c < 48 || c > 57) {
            delete offset;
            throw InvalidCommand("Invalid");
        }
    }
    int quantity = stringToInt(quantityString);

    // read the book data
    Book book;
    _books.seekg(*offset);
    _books.read(reinterpret_cast<char*>(&book), sizeof(Book));
    if (quantity > book.quantity) {
        delete offset;
        throw InvalidCommand("Invalid");
    }
    book.quantity -= quantity;
    std::cout << std::fixed << std::setprecision(2) << quantity * book.price << std::endl;
    _books.seekp(*offset);
    _books.write(reinterpret_cast<const char*>(&book), sizeof(Book));

    // add logs
    Log log(Log::buy, quantity * book.price, quantity, true,
            UserID(loggingStatus.getID()), *offset,
            string_t(), loggingStatus.getPriority());
    logGroup.addLog(log);
    FinanceLog financeLog{quantity * book.price, true};
    logGroup.addFinanceLog(financeLog);
    delete offset;
}

void BookGroup::importBook(TokenScanner& line, const LoggingSituation& loggingStatus, LogGroup& logGroup)
{
    if (loggingStatus.getPriority() < 3) throw InvalidCommand("Invalid");

    if (loggingStatus.getSelected() == -1) throw InvalidCommand("Invalid");

    // read the quantity
    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");
    string_t quantityString = line.nextToken();
    int quantity = stringToInt(quantityString);

    // read the total cost
    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");
    string_t totalCostString = line.nextToken();
    if (line.hasMoreToken()) throw InvalidCommand("Invalid");
    double totalCost = stringToDouble(totalCostString);

    // read the book data
    Book book;
    _books.seekg(loggingStatus.getSelected());
    _books.read(reinterpret_cast<char*>(&book), sizeof(Book));
    book.quantity += quantity;
    _books.seekp(loggingStatus.getSelected());
    _books.write(reinterpret_cast<const char*>(&book), sizeof(Book));

    // add logs
    Log log(Log::import, totalCost, quantity, false,
            UserID(loggingStatus.getID()), loggingStatus.getSelected(),
            string_t(), loggingStatus.getPriority());
    logGroup.addLog(log);
    FinanceLog financeLog{totalCost, false};
    logGroup.addFinanceLog(financeLog);
}

void BookGroup::select(TokenScanner& line, LoggingSituation& loggingStatus, LogGroup& logGroup)
{
    if (loggingStatus.getPriority() < 3) throw InvalidCommand("Invalid");

    // read and check the isbn
    if (!line.hasMoreToken()) throw InvalidCommand("Invalid");
    string_t ISBNString = line.nextToken();
    if (line.hasMoreToken() || !validISBN(ISBNString)) throw InvalidCommand("Invalid");
    ISBN isbn(ISBNString);
    int* offsetPtr = _isbn_book_map.get(isbn);
    int offset;
    if (offsetPtr == nullptr) { // no such book
        _books.seekp(0, std::ios::end);
        offset = _books.tellp();
        _isbn_book_map.insert(isbn, offset);
        Book book(ISBNString);
        _books.write(reinterpret_cast<const char*>(&book), sizeof(Book));

        Log log(Log::create, 0, 0, true,
                UserID(loggingStatus.getID()), offset,
                string_t(), loggingStatus.getPriority());
        logGroup.addLog(log);
    } else {
        offset = *offsetPtr;
    }

    loggingStatus.select(offset);
}

void BookGroup::flush()
{
    _books.flush();
    _isbn_book_map.flush();
    _author_book_map.flush();
    _keywords_book_map.flush();
    _name_book_map.flush();
}

bool validISBN(const string_t& ISBN)
{
    if (ISBN.empty() || ISBN.length() > 20) return false;
    for (char_t c : ISBN) {
        if (c < 33 || c > 126) return false;
    }
    return true;
}

bool validBookName(const string_t& name)
{
    if (name.empty() || name.length() > 60) return false;
    for (char_t c : name) {
        if (c < 33 || c > 126 || c == '\"') return false;
    }
    return true;
}

bool validAuthor(const string_t& author)
{
    if (author.empty() || author.length() > 60) return false;
    for (char_t c : author) {
        if (c < 33 || c > 126 || c == '\"') return false;
    }
    return true;
}

bool validKeyword(const string_t& keyword)
{
    if (keyword.empty() || keyword.length() > 60) return false;
    for (char_t c : keyword) {
        if (c < 33 || c > 126 || c == '\"') return false;
    }
    return true;
}

bool validKeywords(const string_t& keywords)
{
    if (keywords.empty() || keywords.length() > 60) return false;
    TokenScanner keywordSeparator(keywords, '|', TokenScanner::single);
    string_t keyword;
    std::set<string_t> keywordSet;
    while (keywordSeparator.hasMoreToken()) {
        keyword = keywordSeparator.nextToken();
        if (!validKeyword(keyword)) return false;
        if (keywordSet.count(keyword)) return false;
        keywordSet.insert(keyword);
    }
    return true;
}

bool validPrice(const string_t& price)
{
    if (price.empty() || price.length() > 13) return false;
    bool point = false;
    for (char_t c : price) {
        if (c == '.') {
            if (point) return false;
            point = true;
        } else {
            if (c < 48 || c > 57) return false;
        }
    }
    return true;
}

BookParameter processParameter(const string_t& token)
{
    if (token.length() < 2 || token[0] != '-') throw InvalidCommand("Invalid");

    if (token[1] == 'I') {
        if (token.length() < 7) throw InvalidCommand("Invalid");
        string_t checker = token.substr(1, 5);
        if (checker != "ISBN=") throw InvalidCommand("Invalid");

        string_t ISBN = token.substr(6);
        if (!validISBN(ISBN)) throw InvalidCommand("Invalid");

        return BookParameter{isbn, ISBN};

    } else if (token[1] == 'n') {
        if (token.length() < 9) throw InvalidCommand("Invalid");
        string_t checker = token.substr(1, 6);
        if (checker != "name=\"" || token[token.length() - 1] != '\"') throw InvalidCommand("Invalid");

        string_t bookName = token.substr(7, token.length() - 8);
        if (!validBookName(bookName)) throw InvalidCommand("Invalid");

        return BookParameter{name, bookName};

    } else if (token[1] == 'a') {
        if (token.length() < 11) throw InvalidCommand("Invalid");
        string_t checker = token.substr(1, 8);
        if (checker != "author=\"" || token[token.length() - 1] != '\"') throw InvalidCommand("Invalid");

        string_t bookAuthor = token.substr(9, token.length() - 10);
        if (!validAuthor(bookAuthor)) throw InvalidCommand("Invalid");

        return BookParameter{author, bookAuthor};

    } else if (token[1] == 'k') {
        if (token.length() < 12) throw InvalidCommand("Invalid");
        string_t checker = token.substr(1, 9);
        if (checker != "keyword=\"" || token[token.length() - 1] != '\"') throw InvalidCommand("Invalid");

        string_t bookKeywords = token.substr(10, token.length() - 11);
        if (!validKeywords(bookKeywords)) throw InvalidCommand("Invalid");

        return BookParameter{keywords, bookKeywords};

    } else if (token[1] == 'p') {
        if (token.length() < 8) throw InvalidCommand("Invalid");
        string_t checker = token.substr(1, 6);
        if (checker != "price=") throw InvalidCommand("Invalid");

        string_t bookPrice = token.substr(7);
        if (!validPrice(bookPrice)) throw InvalidCommand("Invalid");

        return BookParameter{price, bookPrice};

    } else {
        throw InvalidCommand("Invalid");
    }
}