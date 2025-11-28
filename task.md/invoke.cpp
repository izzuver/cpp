#include <utility>

template<typename Func, typename... Args>
auto invoke(Func&& func, Args&&... args) 
    -> decltype(std::forward<Func>(func)(std::forward<Args>(args)...)) {
    return std::forward<Func>(func)(std::forward<Args>(args)...);
}