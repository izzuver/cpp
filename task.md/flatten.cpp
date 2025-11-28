#include <iostream>

template<typename T>
void flatten(const T& value, std::ostream& out) {
    out << value << ' ';
}

template<typename T>
void flatten(const Array<T>& array, std::ostream& out) {
    for (const auto& element : array) {
        flatten(element, out);
    }
}