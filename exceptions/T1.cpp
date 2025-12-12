#include <iostream>
#include <exception>
#include <string>

class MathException : public std::exception
{
public:
    MathException() noexcept : msg_("math exception")
    {}

    explicit MathException(const std::string& msg) noexcept : msg_(msg)
    {}

    const char* what() const noexcept override
    {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

int divide(int x, int y)
{
    if (y == 0) {
        throw MathException("division by zero");
    }
    return x / y;
}

int main()
{
    try {
        int a = divide(10, 2);
        std::cout << "10 / 2 = " << a << '\n';
    } catch (const MathException& e) {
        std::cout << "Ошибка при 10 / 2: " << e.what() << '\n';
    }

    try {
        int b = divide(10, 0);
        std::cout << "10 / 0 = " << b << '\n';
    } catch (const MathException& e) {
        std::cout << "Ошибка при 10 / 0: " << e.what() << '\n';
    }

    return 0;
}