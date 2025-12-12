#include <iostream>
#include <string>
#include <sstream>
#include <exception>

class bad_from_string : public std::exception
{
public:
    bad_from_string() noexcept : msg_("bad from_string")
    {}

    explicit bad_from_string(const std::string& msg) noexcept : msg_(msg)
    {}

    explicit bad_from_string(const char* msg) noexcept : msg_(msg ? msg : "")
    {}

    const char* what() const noexcept override
    {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

template<class T>
T from_string(std::string const& s)
{
    std::istringstream is(s);
    is >> std::noskipws;

    T value;

    if (!(is >> value)) {
        throw bad_from_string("cannot convert from string: \"" + s + "\"");
    }

    char c;
    if (is >> c) {
        throw bad_from_string("trailing characters in string: \"" + s + "\"");
    }

    return value;
}

int main()
{
    using std::cout;
    using std::string;

    string s1("123");
    cout << "s1 = \"" << s1 << "\"\n";
    try {
        int    a1 = from_string<int>   (s1);
        double b1 = from_string<double>(s1);
        string c1 = from_string<string>(s1);

        cout << "a1 (int)    = " << a1 << '\n';
        cout << "b1 (double) = " << b1 << '\n';
        cout << "c1 (string) = \"" << c1 << "\"\n";
    } catch (const bad_from_string& e) {
        cout << "Исключение для s1: " << e.what() << "\n\n";
    }
    cout << "\n";

    string s2("12.3");
    cout << "s2 = \"" << s2 << "\"\n";
    try {
        int a2 = from_string<int>(s2);
        cout << "a2 (int)    = " << a2 << '\n';
    } catch (const bad_from_string& e) {
        cout << "a2 (int): исключение: " << e.what() << '\n';
    }

    try {
        double b2 = from_string<double>(s2);
        cout << "b2 (double) = " << b2 << '\n';
    } catch (const bad_from_string& e) {
        cout << "b2 (double): исключение: " << e.what() << '\n';
    }

    try {
        string c2 = from_string<string>(s2); // ок
        cout << "c2 (string) = \"" << c2 << "\"\n";
    } catch (const bad_from_string& e) {
        cout << "c2 (string): исключение: " << e.what() << '\n';
    }
    cout << "\n";

    string s3("abc");
    cout << "s3 = \"" << s3 << "\"\n";
    try {
        int a3 = from_string<int>(s3);
        cout << "a3 (int)    = " << a3 << '\n';
    } catch (const bad_from_string& e) {
        cout << "a3 (int): исключение: " << e.what() << '\n';
    }

    try {
        double b3 = from_string<double>(s3);
        cout << "b3 (double) = " << b3 << '\n';
    } catch (const bad_from_string& e) {
        cout << "b3 (double): исключение: " << e.what() << '\n';
    }

    try {
        string c3 = from_string<string>(s3);
        cout << "c3 (string) = \"" << c3 << "\"\n";
    } catch (const bad_from_string& e) {
        cout << "c3 (string): исключение: " << e.what() << '\n';
    }

    return 0;
}