#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

struct Account {
    string password;
    string username;
    int privilege = 1;
};

struct Book {
    string isbn;
    string name;
    string author;
    string keyword;
    long long price_cents = 0;
    int stock = 0;
};

struct SessionFrame {
    string user_id;
    bool has_selected = false;
    string selected_isbn;
};

class Bookstore {
public:
    Bookstore() {
        accounts_["root"] = Account{"sjtu", "root", 7};
    }

    void run() {
        string line;
        while (getline(cin, line)) {
            if (!processLine(line)) break;
        }
    }

private:
    unordered_map<string, Account> accounts_;
    unordered_map<string, int> login_count_;
    vector<SessionFrame> login_stack_;

    map<string, Book> books_;  // sorted by ISBN
    unordered_map<string, set<string>> name_index_;
    unordered_map<string, set<string>> author_index_;
    unordered_map<string, set<string>> keyword_index_;  // keyword segment -> isbns

    vector<long long> finance_tx_;  // +income, -expense

    static bool isVisibleNoSpaceAscii(char c) {
        unsigned char uc = static_cast<unsigned char>(c);
        return uc >= 33 && uc <= 126;
    }

    static bool isPrintableAsciiAllowSpaceNoQuote(char c) {
        unsigned char uc = static_cast<unsigned char>(c);
        return uc >= 32 && uc <= 126 && c != '"';
    }

    static bool isValidUserID(const string &s) {
        if (s.empty() || s.size() > 30) return false;
        for (char c : s) {
            if (!(isalnum(static_cast<unsigned char>(c)) || c == '_')) return false;
        }
        return true;
    }

    static bool isValidPassword(const string &s) {
        return isValidUserID(s);
    }

    static bool isValidUsername(const string &s) {
        if (s.empty() || s.size() > 30) return false;
        for (char c : s) {
            if (!isVisibleNoSpaceAscii(c)) return false;
        }
        return true;
    }

    static bool isValidISBN(const string &s) {
        if (s.empty() || s.size() > 20) return false;
        for (char c : s) {
            if (!isVisibleNoSpaceAscii(c)) return false;
        }
        return true;
    }

    static bool isValidBookText(const string &s) {
        if (s.empty() || s.size() > 60) return false;
        for (char c : s) {
            if (!isPrintableAsciiAllowSpaceNoQuote(c)) return false;
        }
        return true;
    }

    static bool parseNonNegativeInt(const string &s, int max_len, int &out) {
        if (s.empty() || static_cast<int>(s.size()) > max_len) return false;
        long long val = 0;
        for (char c : s) {
            if (!isdigit(static_cast<unsigned char>(c))) return false;
            val = val * 10 + (c - '0');
            if (val > 2147483647LL) return false;
        }
        out = static_cast<int>(val);
        return true;
    }

    static bool parsePositiveInt(const string &s, int max_len, int &out) {
        if (!parseNonNegativeInt(s, max_len, out)) return false;
        return out > 0;
    }

    static bool parseMoneyCents(const string &s, long long &out_cents) {
        if (s.empty() || s.size() > 13) return false;
        int dots = 0;
        for (char c : s) {
            if (!(isdigit(static_cast<unsigned char>(c)) || c == '.')) return false;
            if (c == '.') ++dots;
        }
        if (dots > 1) return false;

        size_t pos = s.find('.');
        string int_part = (pos == string::npos) ? s : s.substr(0, pos);
        string frac_part = (pos == string::npos) ? "" : s.substr(pos + 1);

        if (int_part.empty()) return false;
        if (frac_part.size() > 2) return false;

        long long integer = 0;
        for (char c : int_part) {
            integer = integer * 10 + (c - '0');
            if (integer > 1000000000000LL) return false;
        }

        long long frac = 0;
        if (!frac_part.empty()) {
            for (char c : frac_part) frac = frac * 10 + (c - '0');
            if (frac_part.size() == 1) frac *= 10;
        }

        out_cents = integer * 100 + frac;
        return true;
    }

    static vector<string> splitByPipe(const string &s) {
        vector<string> parts;
        string cur;
        for (char c : s) {
            if (c == '|') {
                parts.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        parts.push_back(cur);
        return parts;
    }

    static bool validateKeywordField(const string &kw, bool require_unique, vector<string> *segments_out = nullptr) {
        if (!isValidBookText(kw)) return false;
        auto segs = splitByPipe(kw);
        for (const string &seg : segs) {
            if (seg.empty()) return false;
        }
        if (require_unique) {
            unordered_set<string> seen;
            for (const string &seg : segs) {
                if (!seen.insert(seg).second) return false;
            }
        }
        if (segments_out) *segments_out = std::move(segs);
        return true;
    }

    static string trim(const string &s) {
        size_t l = 0;
        while (l < s.size() && s[l] == ' ') ++l;
        size_t r = s.size();
        while (r > l && s[r - 1] == ' ') --r;
        return s.substr(l, r - l);
    }

    static bool tokenize(const string &line, vector<string> &tokens) {
        tokens.clear();
        bool in_quote = false;
        string cur;
        for (char c : line) {
            if (c == '"') {
                in_quote = !in_quote;
                cur.push_back(c);
            } else if (c == ' ' && !in_quote) {
                if (!cur.empty()) {
                    tokens.push_back(cur);
                    cur.clear();
                }
            } else {
                cur.push_back(c);
            }
        }
        if (in_quote) return false;
        if (!cur.empty()) tokens.push_back(cur);
        return true;
    }

    static string moneyToString(long long cents) {
        ostringstream oss;
        oss << (cents / 100) << '.' << setw(2) << setfill('0') << (cents % 100);
        return oss.str();
    }

    int currentPrivilege() const {
        if (login_stack_.empty()) return 0;
        auto it = accounts_.find(login_stack_.back().user_id);
        if (it == accounts_.end()) return 0;
        return it->second.privilege;
    }

    void printInvalid() const {
        cout << "Invalid\n";
    }

    bool requirePrivilege(int need) {
        if (currentPrivilege() < need) {
            printInvalid();
            return false;
        }
        return true;
    }

    void indexBookAdd(const Book &b) {
        name_index_[b.name].insert(b.isbn);
        author_index_[b.author].insert(b.isbn);
        if (!b.keyword.empty()) {
            auto segs = splitByPipe(b.keyword);
            for (const auto &seg : segs) {
                keyword_index_[seg].insert(b.isbn);
            }
        }
    }

    void eraseKey(unordered_map<string, set<string>> &idx, const string &k, const string &isbn) {
        auto it = idx.find(k);
        if (it == idx.end()) return;
        it->second.erase(isbn);
        if (it->second.empty()) idx.erase(it);
    }

    void indexBookRemove(const Book &b) {
        eraseKey(name_index_, b.name, b.isbn);
        eraseKey(author_index_, b.author, b.isbn);
        if (!b.keyword.empty()) {
            auto segs = splitByPipe(b.keyword);
            for (const auto &seg : segs) {
                eraseKey(keyword_index_, seg, b.isbn);
            }
        }
    }

    void printBook(const Book &b) const {
        cout << b.isbn << '\t' << b.name << '\t' << b.author << '\t' << b.keyword << '\t'
             << moneyToString(b.price_cents) << '\t' << b.stock << '\n';
    }

    bool processLine(const string &raw_line) {
        string line = trim(raw_line);
        if (line.empty()) return true;

        vector<string> t;
        if (!tokenize(line, t) || t.empty()) {
            printInvalid();
            return true;
        }

        const string &cmd = t[0];

        if (cmd == "quit" || cmd == "exit") {
            if (t.size() != 1) {
                printInvalid();
                return true;
            }
            return false;
        }

        if (cmd == "su") {
            if (!(t.size() == 2 || t.size() == 3)) {
                printInvalid();
                return true;
            }
            const string &uid = t[1];
            if (!isValidUserID(uid)) {
                printInvalid();
                return true;
            }
            auto it = accounts_.find(uid);
            if (it == accounts_.end()) {
                printInvalid();
                return true;
            }

            if (t.size() == 2) {
                if (currentPrivilege() <= it->second.privilege) {
                    printInvalid();
                    return true;
                }
            } else {
                const string &pwd = t[2];
                if (!isValidPassword(pwd) || pwd != it->second.password) {
                    printInvalid();
                    return true;
                }
            }

            login_stack_.push_back(SessionFrame{uid, false, ""});
            ++login_count_[uid];
            return true;
        }

        if (cmd == "logout") {
            if (t.size() != 1 || !requirePrivilege(1)) return true;
            if (login_stack_.empty()) {
                printInvalid();
                return true;
            }
            string uid = login_stack_.back().user_id;
            login_stack_.pop_back();
            auto it = login_count_.find(uid);
            if (it != login_count_.end()) {
                if (--it->second <= 0) login_count_.erase(it);
            }
            return true;
        }

        if (cmd == "register") {
            if (t.size() != 4) {
                printInvalid();
                return true;
            }
            const string &uid = t[1], &pwd = t[2], &uname = t[3];
            if (!isValidUserID(uid) || !isValidPassword(pwd) || !isValidUsername(uname)) {
                printInvalid();
                return true;
            }
            if (accounts_.count(uid)) {
                printInvalid();
                return true;
            }
            accounts_[uid] = Account{pwd, uname, 1};
            return true;
        }

        if (cmd == "passwd") {
            if (!requirePrivilege(1)) return true;
            if (!(t.size() == 3 || t.size() == 4)) {
                printInvalid();
                return true;
            }
            const string &uid = t[1];
            if (!isValidUserID(uid)) {
                printInvalid();
                return true;
            }
            auto it = accounts_.find(uid);
            if (it == accounts_.end()) {
                printInvalid();
                return true;
            }

            string new_pwd;
            if (t.size() == 3) {
                if (currentPrivilege() != 7) {
                    printInvalid();
                    return true;
                }
                new_pwd = t[2];
                if (!isValidPassword(new_pwd)) {
                    printInvalid();
                    return true;
                }
            } else {
                const string &cur_pwd = t[2];
                new_pwd = t[3];
                if (!isValidPassword(cur_pwd) || !isValidPassword(new_pwd) || cur_pwd != it->second.password) {
                    printInvalid();
                    return true;
                }
            }

            it->second.password = new_pwd;
            return true;
        }

        if (cmd == "useradd") {
            if (!requirePrivilege(3)) return true;
            if (t.size() != 5) {
                printInvalid();
                return true;
            }
            const string &uid = t[1], &pwd = t[2], &priv_s = t[3], &uname = t[4];
            if (!isValidUserID(uid) || !isValidPassword(pwd) || !isValidUsername(uname)) {
                printInvalid();
                return true;
            }
            int priv = 0;
            if (!parseNonNegativeInt(priv_s, 1, priv) || (priv != 1 && priv != 3 && priv != 7) || priv >= currentPrivilege()) {
                printInvalid();
                return true;
            }
            if (accounts_.count(uid)) {
                printInvalid();
                return true;
            }
            accounts_[uid] = Account{pwd, uname, priv};
            return true;
        }

        if (cmd == "delete") {
            if (!requirePrivilege(7)) return true;
            if (t.size() != 2 || !isValidUserID(t[1])) {
                printInvalid();
                return true;
            }
            const string &uid = t[1];
            if (!accounts_.count(uid) || login_count_.count(uid)) {
                printInvalid();
                return true;
            }
            accounts_.erase(uid);
            return true;
        }

        if (cmd == "show") {
            if (t.size() >= 2 && t[1] == "finance") {
                if (!requirePrivilege(7)) return true;
                if (!(t.size() == 2 || t.size() == 3)) {
                    printInvalid();
                    return true;
                }
                const bool has_count = (t.size() == 3);
                int cnt = static_cast<int>(finance_tx_.size());
                if (has_count) {
                    if (!parseNonNegativeInt(t[2], 10, cnt)) {
                        printInvalid();
                        return true;
                    }
                    if (cnt > static_cast<int>(finance_tx_.size())) {
                        printInvalid();
                        return true;
                    }
                    if (cnt == 0) {
                        cout << '\n';
                        return true;
                    }
                }

                long long income = 0, expense = 0;
                int n = static_cast<int>(finance_tx_.size());
                for (int i = n - cnt; i < n; ++i) {
                    if (finance_tx_[i] >= 0) income += finance_tx_[i];
                    else expense += -finance_tx_[i];
                }
                cout << "+ " << moneyToString(income) << " - " << moneyToString(expense) << '\n';
                return true;
            }

            if (!requirePrivilege(1)) return true;
            if (!(t.size() == 1 || t.size() == 2)) {
                printInvalid();
                return true;
            }

            vector<string> isbns;
            if (t.size() == 1) {
                isbns.reserve(books_.size());
                for (const auto &kv : books_) isbns.push_back(kv.first);
            } else {
                const string &arg = t[1];
                if (arg.rfind("-ISBN=", 0) == 0) {
                    string v = arg.substr(6);
                    if (!isValidISBN(v)) {
                        printInvalid();
                        return true;
                    }
                    if (books_.count(v)) isbns.push_back(v);
                } else if (arg.rfind("-name=\"", 0) == 0 && arg.size() >= 8 && arg.back() == '"') {
                    string v = arg.substr(7, arg.size() - 8);
                    if (!isValidBookText(v)) {
                        printInvalid();
                        return true;
                    }
                    auto it = name_index_.find(v);
                    if (it != name_index_.end()) {
                        for (const auto &x : it->second) isbns.push_back(x);
                    }
                } else if (arg.rfind("-author=\"", 0) == 0 && arg.size() >= 10 && arg.back() == '"') {
                    string v = arg.substr(9, arg.size() - 10);
                    if (!isValidBookText(v)) {
                        printInvalid();
                        return true;
                    }
                    auto it = author_index_.find(v);
                    if (it != author_index_.end()) {
                        for (const auto &x : it->second) isbns.push_back(x);
                    }
                } else if (arg.rfind("-keyword=\"", 0) == 0 && arg.size() >= 11 && arg.back() == '"') {
                    string v = arg.substr(10, arg.size() - 11);
                    if (!isValidBookText(v) || v.find('|') != string::npos) {
                        printInvalid();
                        return true;
                    }
                    auto it = keyword_index_.find(v);
                    if (it != keyword_index_.end()) {
                        for (const auto &x : it->second) isbns.push_back(x);
                    }
                } else {
                    printInvalid();
                    return true;
                }
            }

            if (isbns.empty()) {
                cout << '\n';
                return true;
            }
            for (const auto &isbn : isbns) {
                auto it = books_.find(isbn);
                if (it != books_.end()) printBook(it->second);
            }
            return true;
        }

        if (cmd == "buy") {
            if (!requirePrivilege(1)) return true;
            if (t.size() != 3 || !isValidISBN(t[1])) {
                printInvalid();
                return true;
            }
            int qty = 0;
            if (!parsePositiveInt(t[2], 10, qty)) {
                printInvalid();
                return true;
            }
            auto it = books_.find(t[1]);
            if (it == books_.end() || it->second.stock < qty) {
                printInvalid();
                return true;
            }
            it->second.stock -= qty;
            long long amount = it->second.price_cents * qty;
            finance_tx_.push_back(amount);
            cout << moneyToString(amount) << '\n';
            return true;
        }

        if (cmd == "select") {
            if (!requirePrivilege(3)) return true;
            if (t.size() != 2 || !isValidISBN(t[1]) || login_stack_.empty()) {
                printInvalid();
                return true;
            }
            const string &isbn = t[1];
            auto it = books_.find(isbn);
            if (it == books_.end()) {
                books_[isbn] = Book{isbn, "", "", "", 0, 0};
                indexBookAdd(books_[isbn]);
            }
            login_stack_.back().has_selected = true;
            login_stack_.back().selected_isbn = isbn;
            return true;
        }

        if (cmd == "modify") {
            if (!requirePrivilege(3)) return true;
            if (t.size() < 2 || login_stack_.empty() || !login_stack_.back().has_selected) {
                printInvalid();
                return true;
            }
            string cur_isbn = login_stack_.back().selected_isbn;
            auto it_book = books_.find(cur_isbn);
            if (it_book == books_.end()) {
                printInvalid();
                return true;
            }

            bool has_isbn = false, has_name = false, has_author = false, has_keyword = false, has_price = false;
            string new_isbn, new_name, new_author, new_keyword;
            long long new_price = 0;

            for (size_t i = 1; i < t.size(); ++i) {
                const string &arg = t[i];
                if (arg.rfind("-ISBN=", 0) == 0) {
                    if (has_isbn) {
                        printInvalid();
                        return true;
                    }
                    has_isbn = true;
                    new_isbn = arg.substr(6);
                    if (!isValidISBN(new_isbn)) {
                        printInvalid();
                        return true;
                    }
                } else if (arg.rfind("-name=\"", 0) == 0 && arg.size() >= 8 && arg.back() == '"') {
                    if (has_name) {
                        printInvalid();
                        return true;
                    }
                    has_name = true;
                    new_name = arg.substr(7, arg.size() - 8);
                    if (!isValidBookText(new_name)) {
                        printInvalid();
                        return true;
                    }
                } else if (arg.rfind("-author=\"", 0) == 0 && arg.size() >= 10 && arg.back() == '"') {
                    if (has_author) {
                        printInvalid();
                        return true;
                    }
                    has_author = true;
                    new_author = arg.substr(9, arg.size() - 10);
                    if (!isValidBookText(new_author)) {
                        printInvalid();
                        return true;
                    }
                } else if (arg.rfind("-keyword=\"", 0) == 0 && arg.size() >= 11 && arg.back() == '"') {
                    if (has_keyword) {
                        printInvalid();
                        return true;
                    }
                    has_keyword = true;
                    new_keyword = arg.substr(10, arg.size() - 11);
                    if (!validateKeywordField(new_keyword, true)) {
                        printInvalid();
                        return true;
                    }
                } else if (arg.rfind("-price=", 0) == 0) {
                    if (has_price) {
                        printInvalid();
                        return true;
                    }
                    has_price = true;
                    if (!parseMoneyCents(arg.substr(7), new_price)) {
                        printInvalid();
                        return true;
                    }
                } else {
                    printInvalid();
                    return true;
                }
            }

            Book old_book = it_book->second;
            Book updated = old_book;
            if (has_isbn) updated.isbn = new_isbn;
            if (has_name) updated.name = new_name;
            if (has_author) updated.author = new_author;
            if (has_keyword) updated.keyword = new_keyword;
            if (has_price) updated.price_cents = new_price;

            if (has_isbn) {
                if (new_isbn == old_book.isbn) {
                    printInvalid();
                    return true;
                }
                if (books_.count(new_isbn)) {
                    printInvalid();
                    return true;
                }
            }

            indexBookRemove(old_book);
            books_.erase(old_book.isbn);
            books_[updated.isbn] = updated;
            indexBookAdd(updated);

            if (has_isbn) {
                for (auto &frame : login_stack_) {
                    if (frame.has_selected && frame.selected_isbn == old_book.isbn) {
                        frame.selected_isbn = updated.isbn;
                    }
                }
            }

            return true;
        }

        if (cmd == "import") {
            if (!requirePrivilege(3)) return true;
            if (t.size() != 3 || login_stack_.empty() || !login_stack_.back().has_selected) {
                printInvalid();
                return true;
            }
            int qty = 0;
            long long cost = 0;
            if (!parsePositiveInt(t[1], 10, qty) || !parseMoneyCents(t[2], cost) || cost <= 0) {
                printInvalid();
                return true;
            }
            auto it = books_.find(login_stack_.back().selected_isbn);
            if (it == books_.end()) {
                printInvalid();
                return true;
            }
            it->second.stock += qty;
            finance_tx_.push_back(-cost);
            return true;
        }

        if (cmd == "report") {
            if (!requirePrivilege(7)) return true;
            if (t.size() != 2 || (t[1] != "finance" && t[1] != "employee")) {
                printInvalid();
                return true;
            }
            cout << '\n';
            return true;
        }

        if (cmd == "log") {
            if (!requirePrivilege(7)) return true;
            if (t.size() != 1) {
                printInvalid();
                return true;
            }
            cout << '\n';
            return true;
        }

        printInvalid();
        return true;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Bookstore bs;
    bs.run();
    return 0;
}
