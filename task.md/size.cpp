template<typename T, size_t N>
constexpr size_t size(const Array<T, N>&) {
    return N;
}