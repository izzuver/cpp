#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <numeric>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

template <typename T, typename Compare = std::less<T>>
class SkipList {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

private:
    static constexpr size_type kMaxLevel = 16;

    struct NodeBase {
        std::array<NodeBase*, kMaxLevel> next;
        size_type level;
        NodeBase* prev;

        explicit NodeBase(size_type lvl)
            : next{}, level(lvl), prev(nullptr) {
            next.fill(nullptr);
        }
    };

    struct Node : NodeBase {
        T value;

        template <typename U>
        Node(size_type lvl, U&& v)
            : NodeBase(lvl), value(std::forward<U>(v)) {}
    };

public:
    template <bool IsConst>
    class basic_iterator {
    private:
        using node_base_pointer =
            std::conditional_t<IsConst, const NodeBase*, NodeBase*>;
        using node_pointer =
            std::conditional_t<IsConst, const Node*, Node*>;

        node_base_pointer node_;
        const SkipList* owner_;

        basic_iterator(node_base_pointer node, const SkipList* owner)
            : node_(node), owner_(owner) {}

        friend class SkipList;
        template <bool>
        friend class basic_iterator;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;

        basic_iterator() : node_(nullptr), owner_(nullptr) {}

        template <bool B = IsConst, std::enable_if_t<B, int> = 0>
        basic_iterator(const basic_iterator<false>& other)
            : node_(other.node_), owner_(other.owner_) {}

        reference operator*() const {
            return static_cast<node_pointer>(node_)->value;
        }

        pointer operator->() const {
            return &static_cast<node_pointer>(node_)->value;
        }

        basic_iterator& operator++() {
            if (node_ != nullptr) {
                node_ = node_->next[0];
            }
            return *this;
        }

        basic_iterator operator++(int) {
            basic_iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        basic_iterator& operator--() {
            if (node_ == nullptr) {
                node_ = owner_ ? owner_->tail_ : nullptr;
            } else {
                node_ = node_->prev;
            }
            return *this;
        }

        basic_iterator operator--(int) {
            basic_iterator tmp(*this);
            --(*this);
            return tmp;
        }

        template <bool OtherConst>
        bool operator==(const basic_iterator<OtherConst>& other) const {
            return node_ == other.node_ && owner_ == other.owner_;
        }

        template <bool OtherConst>
        bool operator!=(const basic_iterator<OtherConst>& other) const {
            return !(*this == other);
        }
    };

    using iterator = basic_iterator<false>;
    using const_iterator = basic_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    NodeBase* head_;
    NodeBase* tail_;
    size_type size_;
    size_type current_level_;
    Compare comp_;
    std::mt19937 gen_;
    double promote_probability_;

private:
    const T& value_of(const NodeBase* node) const {
        return static_cast<const Node*>(node)->value;
    }

    bool equivalent(const T& lhs, const T& rhs) const {
        return !comp_(lhs, rhs) && !comp_(rhs, lhs);
    }

    size_type random_level() {
        std::bernoulli_distribution coin(promote_probability_);
        size_type lvl = 1;
        while (lvl < kMaxLevel && coin(gen_)) {
            ++lvl;
        }
        return lvl;
    }

    NodeBase* find_predecessors(const T& key,
                                std::array<NodeBase*, kMaxLevel>& update) {
        NodeBase* current = head_;

        for (size_type level = current_level_; level-- > 0;) {
            while (current->next[level] != nullptr &&
                   comp_(value_of(current->next[level]), key)) {
                current = current->next[level];
            }
            update[level] = current;
        }

        return current->next[0];
    }

    NodeBase* lower_bound_node(const T& key) {
        NodeBase* current = head_;

        for (size_type level = current_level_; level-- > 0;) {
            while (current->next[level] != nullptr &&
                   comp_(value_of(current->next[level]), key)) {
                current = current->next[level];
            }
        }

        return current->next[0];
    }

    const NodeBase* lower_bound_node(const T& key) const {
        const NodeBase* current = head_;

        for (size_type level = current_level_; level-- > 0;) {
            while (current->next[level] != nullptr &&
                   comp_(value_of(current->next[level]), key)) {
                current = current->next[level];
            }
        }

        return current->next[0];
    }

    NodeBase* upper_bound_node(const T& key) {
        NodeBase* current = head_;

        for (size_type level = current_level_; level-- > 0;) {
            while (current->next[level] != nullptr &&
                   !comp_(key, value_of(current->next[level]))) {
                current = current->next[level];
            }
        }

        return current->next[0];
    }

    const NodeBase* upper_bound_node(const T& key) const {
        const NodeBase* current = head_;

        for (size_type level = current_level_; level-- > 0;) {
            while (current->next[level] != nullptr &&
                   !comp_(key, value_of(current->next[level]))) {
                current = current->next[level];
            }
        }

        return current->next[0];
    }

    template <typename U>
    std::pair<iterator, bool> insert_impl(U&& value) {
        const T& key = value;

        std::array<NodeBase*, kMaxLevel> update{};
        update.fill(nullptr);

        NodeBase* candidate = find_predecessors(key, update);

        if (candidate != nullptr && equivalent(value_of(candidate), key)) {
            return { iterator(candidate, this), false };
        }

        const size_type node_level = random_level();
        Node* new_node = new Node(node_level, std::forward<U>(value));

        if (node_level > current_level_) {
            for (size_type i = current_level_; i < node_level; ++i) {
                update[i] = head_;
            }
            current_level_ = node_level;
        }

        for (size_type i = 0; i < node_level; ++i) {
            new_node->next[i] = update[i]->next[i];
            update[i]->next[i] = new_node;
        }

        new_node->prev = (update[0] == head_) ? nullptr : update[0];

        if (new_node->next[0] != nullptr) {
            new_node->next[0]->prev = new_node;
        } else {
            tail_ = new_node;
        }

        if (size_ == 0) {
            tail_ = new_node;
        }

        ++size_;
        return { iterator(new_node, this), true };
    }

public:
    SkipList()
        : SkipList(Compare(), 0.5) {}

    explicit SkipList(const Compare& comp, double p = 0.5)
        : head_(new NodeBase(kMaxLevel)),
          tail_(nullptr),
          size_(0),
          current_level_(1),
          comp_(comp),
          gen_(std::random_device{}()),
          promote_probability_(p) {
        assert(p > 0.0 && p < 1.0);
    }

    template <typename InputIt, std::enable_if_t<!std::is_integral_v<InputIt>, int> = 0>
    SkipList(InputIt first, InputIt last, const Compare& comp = Compare(), double p = 0.5)
        : SkipList(comp, p) {
        insert(first, last);
    }

    SkipList(const SkipList& other)
        : SkipList(other.comp_, other.promote_probability_) {
        insert(other.begin(), other.end());
    }

    SkipList(SkipList&& other)
        : SkipList(other.comp_, other.promote_probability_) {
        swap(other);
    }

    ~SkipList() {
        clear();
        delete head_;
    }

    SkipList& operator=(const SkipList& other) {
        if (this != &other) {
            SkipList tmp(other);
            swap(tmp);
        }
        return *this;
    }

    SkipList& operator=(SkipList&& other) {
        if (this != &other) {
            SkipList tmp(std::move(other));
            swap(tmp);
        }
        return *this;
    }

    void swap(SkipList& other) {
        using std::swap;
        swap(head_, other.head_);
        swap(tail_, other.tail_);
        swap(size_, other.size_);
        swap(current_level_, other.current_level_);
        swap(comp_, other.comp_);
        swap(gen_, other.gen_);
        swap(promote_probability_, other.promote_probability_);
    }

    bool empty() const noexcept {
        return size_ == 0;
    }

    size_type size() const noexcept {
        return size_;
    }

    double probability() const noexcept {
        return promote_probability_;
    }

    void clear() noexcept {
        NodeBase* current = head_->next[0];

        while (current != nullptr) {
            NodeBase* next_node = current->next[0];
            delete static_cast<Node*>(current);
            current = next_node;
        }

        head_->next.fill(nullptr);
        tail_ = nullptr;
        size_ = 0;
        current_level_ = 1;
    }

    std::pair<iterator, bool> insert(const T& value) {
        return insert_impl(value);
    }

    std::pair<iterator, bool> insert(T&& value) {
        return insert_impl(std::move(value));
    }

    template <typename InputIt, std::enable_if_t<!std::is_integral_v<InputIt>, int> = 0>
    void insert(InputIt first, InputIt last) {
        for (; first != last; ++first) {
            insert(*first);
        }
    }

    iterator find(const T& key) {
        NodeBase* node = lower_bound_node(key);
        if (node != nullptr && equivalent(value_of(node), key)) {
            return iterator(node, this);
        }
        return end();
    }

    const_iterator find(const T& key) const {
        const NodeBase* node = lower_bound_node(key);
        if (node != nullptr && equivalent(value_of(node), key)) {
            return const_iterator(node, this);
        }
        return end();
    }

    size_type count(const T& key) const {
        return find(key) == end() ? 0 : 1;
    }

    iterator lower_bound(const T& key) {
        return iterator(lower_bound_node(key), this);
    }

    const_iterator lower_bound(const T& key) const {
        return const_iterator(lower_bound_node(key), this);
    }

    iterator upper_bound(const T& key) {
        return iterator(upper_bound_node(key), this);
    }

    const_iterator upper_bound(const T& key) const {
        return const_iterator(upper_bound_node(key), this);
    }

    // Важно: для неверных итераторов поведение не определено.
    iterator erase(iterator pos) {
        NodeBase* target = pos.node_;
        NodeBase* next_node = target->next[0];

        std::array<NodeBase*, kMaxLevel> update{};
        update.fill(nullptr);
        find_predecessors(value_of(target), update);

        for (size_type i = 0; i < current_level_; ++i) {
            if (update[i]->next[i] == target) {
                update[i]->next[i] = target->next[i];
            }
        }

        if (target->next[0] != nullptr) {
            target->next[0]->prev = target->prev;
        } else {
            tail_ = target->prev;
        }

        delete static_cast<Node*>(target);
        --size_;

        while (current_level_ > 1 && head_->next[current_level_ - 1] == nullptr) {
            --current_level_;
        }

        return iterator(next_node, this);
    }

    // Важно: для неверных диапазонов поведение не определено.
    iterator erase(iterator first, iterator last) {
        while (first != last) {
            first = erase(first);
        }
        return last;
    }

    // Доп. удобная перегрузка
    size_type erase(const T& key) {
        iterator it = find(key);
        if (it == end()) {
            return 0;
        }
        erase(it);
        return 1;
    }

    iterator begin() noexcept {
        return iterator(head_->next[0], this);
    }

    const_iterator begin() const noexcept {
        return const_iterator(head_->next[0], this);
    }

    const_iterator cbegin() const noexcept {
        return const_iterator(head_->next[0], this);
    }

    iterator end() noexcept {
        return iterator(nullptr, this);
    }

    const_iterator end() const noexcept {
        return const_iterator(nullptr, this);
    }

    const_iterator cend() const noexcept {
        return const_iterator(nullptr, this);
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    void debug_dump(std::ostream& os) const {
        for (size_type level = current_level_; level-- > 0;) {
            os << "Level " << level << ": ";
            const NodeBase* current = head_->next[level];
            while (current != nullptr) {
                os << value_of(current) << "[" << current->level << "] ";
                current = current->next[level];
            }
            os << '\n';
        }
    }
};

template <typename T, typename Compare>
void swap(SkipList<T, Compare>& lhs, SkipList<T, Compare>& rhs) {
    lhs.swap(rhs);
}

/* =========================================================
   Отладка и тесты
   ========================================================= */

template <typename T, typename Compare>
void print_skiplist(const SkipList<T, Compare>& list, const std::string& name) {
    std::cout << name << " (size = " << list.size()
              << ", p = " << list.probability() << "): ";
    for (const auto& value : list) {
        std::cout << value << ' ';
    }
    std::cout << '\n';
}

template <typename T, typename Compare>
std::vector<T> to_vector(const SkipList<T, Compare>& list) {
    return std::vector<T>(list.begin(), list.end());
}

template <typename T, typename Compare>
void expect_sequence(const SkipList<T, Compare>& list, const std::vector<T>& expected) {
    assert(list.size() == expected.size());
    assert(to_vector(list) == expected);
}

template <typename T, typename CompareA, typename CompareB>
bool same_sequence(const SkipList<T, CompareA>& a, const SkipList<T, CompareB>& b) {
    return to_vector(a) == to_vector(b);
}

template <typename List>
void print_bound_result(const List& list,
                        typename List::const_iterator it,
                        const std::string& label) {
    std::cout << label << " = ";
    if (it == list.end()) {
        std::cout << "end";
    } else {
        std::cout << *it;
    }
    std::cout << '\n';
}

struct GreaterInt {
    bool operator()(int a, int b) const {
        return a > b;
    }
};

struct NoDefault {
    int x;

    explicit NoDefault(int value) : x(value) {}
};

bool operator==(const NoDefault& a, const NoDefault& b) {
    return a.x == b.x;
}

std::ostream& operator<<(std::ostream& os, const NoDefault& value) {
    os << value.x;
    return os;
}

struct NoDefaultLess {
    bool operator()(const NoDefault& a, const NoDefault& b) const {
        return a.x < b.x;
    }
}

;

void test_default_constructor() {
    std::cout << "=== test_default_constructor ===\n";

    SkipList<int> list;
    assert(list.empty());
    assert(list.size() == 0);
    assert(list.begin() == list.end());

    print_skiplist(list, "list");
    std::cout << '\n';
}

void test_insert_and_find() {
    std::cout << "=== test_insert_and_find ===\n";

    SkipList<int> list(std::less<int>{}, 0.5);

    auto r1 = list.insert(5);
    auto r2 = list.insert(3);
    auto r3 = list.insert(8);
    auto r4 = list.insert(5);

    assert(r1.second);
    assert(r2.second);
    assert(r3.second);
    assert(!r4.second);

    expect_sequence(list, {3, 5, 8});

    auto it = list.find(5);
    assert(it != list.end());
    assert(*it == 5);

    assert(list.find(100) == list.end());

    print_skiplist(list, "list");
    std::cout << "find(5): " << *list.find(5) << '\n';
    std::cout << "find(100): end\n\n";
}

void test_count_lower_upper() {
    std::cout << "=== test_count_lower_upper ===\n";

    SkipList<int> list;
    for (int x : {10, 20, 30, 40, 50}) {
        list.insert(x);
    }

    print_skiplist(list, "list");

    assert(list.count(30) == 1);
    assert(list.count(35) == 0);

    auto lb1 = list.lower_bound(25);
    auto lb2 = list.lower_bound(30);
    auto lb3 = list.lower_bound(99);

    auto ub1 = list.upper_bound(30);
    auto ub2 = list.upper_bound(50);

    assert(lb1 != list.end() && *lb1 == 30);
    assert(lb2 != list.end() && *lb2 == 30);
    assert(lb3 == list.end());
    assert(ub1 != list.end() && *ub1 == 40);
    assert(ub2 == list.end());

    std::cout << "count(30) = " << list.count(30) << '\n';
    std::cout << "count(35) = " << list.count(35) << '\n';
    print_bound_result(list, lb1, "lower_bound(25)");
    print_bound_result(list, lb2, "lower_bound(30)");
    print_bound_result(list, lb3, "lower_bound(99)");
    print_bound_result(list, ub1, "upper_bound(30)");
    print_bound_result(list, ub2, "upper_bound(50)");
    std::cout << '\n';
}

void test_range_constructor_and_range_insert() {
    std::cout << "=== test_range_constructor_and_range_insert ===\n";

    std::vector<int> data = {7, 3, 9, 1, 5, 3, 8, 2, 6, 4};

    SkipList<int> list(data.begin(), data.end());
    assert(!list.empty());
    assert(std::is_sorted(list.begin(), list.end()));
    expect_sequence(list, {1, 2, 3, 4, 5, 6, 7, 8, 9});

    print_skiplist(list, "list");
    std::cout << "Internal levels:\n";
    list.debug_dump(std::cout);
    std::cout << '\n';

    std::list<int> other_data = {15, 12, 14, 11, 13};
    SkipList<int> from_list(other_data.begin(), other_data.end());
    expect_sequence(from_list, {11, 12, 13, 14, 15});
    print_skiplist(from_list, "from_list");
    std::cout << '\n';
}

void test_iterators() {
    std::cout << "=== test_iterators ===\n";

    SkipList<int> list;
    for (int x : {1, 2, 3, 4, 5}) {
        list.insert(x);
    }

    auto it = list.begin();
    assert(*it == 1);
    ++it; assert(*it == 2);
    it++; assert(*it == 3);
    --it; assert(*it == 2);
    it--; assert(*it == 1);

    auto last = list.end();
    --last;
    assert(*last == 5);

    assert(static_cast<std::size_t>(std::distance(list.begin(), list.end())) == list.size());

    std::vector<int> reversed;
    for (auto rit = list.rbegin(); rit != list.rend(); ++rit) {
        reversed.push_back(*rit);
    }
    assert((reversed == std::vector<int>{5, 4, 3, 2, 1}));

    std::cout << "Forward : ";
    for (const auto& x : list) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    std::cout << "Reverse : ";
    for (auto rit = list.rbegin(); rit != list.rend(); ++rit) {
        std::cout << *rit << ' ';
    }
    std::cout << "\n\n";
}

void test_erase_single_and_range() {
    std::cout << "=== test_erase_single_and_range ===\n";

    SkipList<int> list;
    for (int x : {1, 2, 3, 4, 5, 6, 7, 8}) {
        list.insert(x);
    }

    auto it2 = list.find(2);
    auto it4 = list.find(4);
    auto it6 = list.find(6);

    auto after_one = list.erase(it4);
    assert(after_one != list.end() && *after_one == 5);

    assert(*it2 == 2);
    assert(*it6 == 6);
    expect_sequence(list, {1, 2, 3, 5, 6, 7, 8});

    auto first = list.lower_bound(3);
    auto last  = list.upper_bound(6);
    auto after_range = list.erase(first, last);
    assert(after_range != list.end() && *after_range == 7);

    expect_sequence(list, {1, 2, 7, 8});
    print_skiplist(list, "after erase");
    std::cout << '\n';
}

void test_clear_and_erase_key() {
    std::cout << "=== test_clear_and_erase_key ===\n";

    SkipList<int> list;
    for (int x : {10, 20, 30, 40}) {
        list.insert(x);
    }

    assert(list.erase(20) == 1);
    assert(list.erase(100) == 0);
    expect_sequence(list, {10, 30, 40});
    print_skiplist(list, "after erase(key)");

    list.clear();
    assert(list.empty());
    assert(list.size() == 0);
    assert(list.begin() == list.end());

    print_skiplist(list, "after clear");
    std::cout << '\n';
}

void test_copy_and_move() {
    std::cout << "=== test_copy_and_move ===\n";

    SkipList<int> original;
    original.insert(10);
    original.insert(20);
    original.insert(30);

    SkipList<int> copied(original);
    assert(same_sequence(original, copied));

    original.insert(40);
    assert(copied.find(40) == copied.end());

    SkipList<int> copy_assigned;
    copy_assigned = original;
    assert(same_sequence(original, copy_assigned));

    SkipList<int> moved(std::move(copied));
    expect_sequence(moved, {10, 20, 30});
    assert(copied.empty());

    SkipList<int> source;
    source.insert(1);
    source.insert(2);
    source.insert(3);

    SkipList<int> target;
    target.insert(100);
    target.insert(200);
    target = std::move(source);

    expect_sequence(target, {1, 2, 3});
    assert(source.empty());

    print_skiplist(original, "original");
    print_skiplist(copy_assigned, "copy_assigned");
    print_skiplist(moved, "moved");
    print_skiplist(target, "target");
    std::cout << '\n';
}

void test_template_instantiation() {
    std::cout << "=== test_template_instantiation ===\n";

    SkipList<std::string> words;
    words.insert("orange");
    words.insert("apple");
    words.insert("banana");
    words.insert("banana");

    expect_sequence(words, {"apple", "banana", "orange"});
    print_skiplist(words, "words");

    auto lb = words.lower_bound("banana");
    auto ub = words.upper_bound("banana");
    print_bound_result(words, lb, "lower_bound(\"banana\")");
    print_bound_result(words, ub, "upper_bound(\"banana\")");
    std::cout << '\n';
}

void test_custom_comparator() {
    std::cout << "=== test_custom_comparator ===\n";

    SkipList<int, GreaterInt> desc(GreaterInt{}, 0.5);
    std::vector<int> data = {4, 1, 7, 3, 5};
    desc.insert(data.begin(), data.end());

    expect_sequence(desc, {7, 5, 4, 3, 1});
    print_skiplist(desc, "desc");

    auto lb = desc.lower_bound(6);
    auto ub = desc.upper_bound(5);

    assert(lb != desc.end() && *lb == 5);
    assert(ub != desc.end() && *ub == 4);

    print_bound_result(desc, lb, "lower_bound(6) in desc");
    print_bound_result(desc, ub, "upper_bound(5) in desc");
    std::cout << '\n';
}

void test_no_default_type() {
    std::cout << "=== test_no_default_type ===\n";
    std::cout << "Тип NoDefault не имеет конструктора по умолчанию.\n";
    std::cout << "Если бы head_ хранил T value напрямую, понадобился бы фиктивный T{},\n";
    std::cout << "и такая реализация не скомпилировалась бы.\n\n";

    SkipList<NoDefault, NoDefaultLess> list(NoDefaultLess{}, 0.5);

    list.insert(NoDefault(10));
    list.insert(NoDefault(5));
    list.insert(NoDefault(7));

    print_skiplist(list, "no_default_list");
    expect_sequence(list, {NoDefault(5), NoDefault(7), NoDefault(10)});

    auto it = list.find(NoDefault(7));
    assert(it != list.end());
    assert(it->x == 7);

    std::cout << "find(NoDefault(7)) = " << it->x << "\n\n";
}

void test_probability_parameter() {
    std::cout << "=== test_probability_parameter ===\n";

    SkipList<int> a(std::less<int>{}, 0.25);
    SkipList<int> b(std::less<int>{}, 0.75);

    for (int x : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) {
        a.insert(x);
        b.insert(x);
    }

    print_skiplist(a, "list_p_025");
    std::cout << "levels for p=0.25:\n";
    a.debug_dump(std::cout);

    print_skiplist(b, "list_p_075");
    std::cout << "levels for p=0.75:\n";
    b.debug_dump(std::cout);

    std::cout << '\n';
}

void test_stl_compatibility() {
    std::cout << "=== test_stl_compatibility ===\n";

    using IntSkipList = SkipList<int>;

    static_assert(
        std::is_same_v<
            std::iterator_traits<IntSkipList::iterator>::iterator_category,
            std::bidirectional_iterator_tag
        >,
        "Iterator must be bidirectional"
    );

    IntSkipList list;
    for (int x : {1,2,3,4,5,6,7,8,9,10}) {
        list.insert(x);
    }

    int sum = std::accumulate(list.begin(), list.end(), 0);
    assert(sum == 55);

    assert(std::find(list.begin(), list.end(), 6) != list.end());
    assert(std::find(list.begin(), list.end(), 100) == list.end());

    std::cout << "sum = " << sum << "\n\n";
}

/* =========================================================
   Benchmark mode
   ========================================================= */

using BenchClock = std::chrono::steady_clock;
static volatile std::size_t g_bench_sink = 0;

std::vector<int> make_keys(std::size_t n, std::uint32_t seed) {
    std::vector<int> keys(n);
    std::iota(keys.begin(), keys.end(), 0);
    std::mt19937 rng(seed);
    std::shuffle(keys.begin(), keys.end(), rng);
    return keys;
}

long long bench_insert_once(std::size_t n, double p, std::uint32_t seed) {
    auto keys = make_keys(n, seed);
    SkipList<int> list(std::less<int>{}, p);

    auto t1 = BenchClock::now();
    for (int x : keys) {
        list.insert(x);
    }
    auto t2 = BenchClock::now();

    g_bench_sink += list.size();

    return std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
}

long long bench_find_once(std::size_t n, double p, std::uint32_t seed) {
    auto keys = make_keys(n, seed);
    SkipList<int> list(std::less<int>{}, p);

    for (int x : keys) {
        list.insert(x);
    }

    std::mt19937 rng(seed + 1);
    std::shuffle(keys.begin(), keys.end(), rng);

    std::size_t hits = 0;

    auto t1 = BenchClock::now();
    for (int x : keys) {
        if (list.find(x) != list.end()) {
            ++hits;
        }
    }
    auto t2 = BenchClock::now();

    g_bench_sink += hits;

    return std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
}

long long bench_erase_once(std::size_t n, double p, std::uint32_t seed) {
    auto keys = make_keys(n, seed);
    SkipList<int> list(std::less<int>{}, p);

    std::vector<SkipList<int>::iterator> its;
    its.reserve(n);

    for (int x : keys) {
        its.push_back(list.insert(x).first);
    }

    std::mt19937 rng(seed + 2);
    std::shuffle(its.begin(), its.end(), rng);

    auto t1 = BenchClock::now();
    for (auto it : its) {
        list.erase(it);
    }
    auto t2 = BenchClock::now();

    g_bench_sink += list.size();

    return std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
}

int run_bench_mode(int argc, char** argv) {
    if (argc != 6) {
        std::cerr << "Usage:\n";
        std::cerr << "  " << argv[0] << " --bench <insert|find|erase> <n> <p> <trials>\n";
        return 1;
    }

    std::string op = argv[2];
    std::size_t n = static_cast<std::size_t>(std::stoull(argv[3]));
    double p = std::stod(argv[4]);
    int trials = std::stoi(argv[5]);

    if (!(p > 0.0 && p < 1.0)) {
        std::cerr << "p must be in (0, 1)\n";
        return 1;
    }

    if (n == 0 || trials <= 0) {
        std::cerr << "n and trials must be positive\n";
        return 1;
    }

    std::cout << "operation,n,p,trial,total_ns,avg_ns_per_op\n";

    for (int t = 0; t < trials; ++t) {
        std::uint32_t seed = 123456789u
                           + static_cast<std::uint32_t>(n * 17u)
                           + static_cast<std::uint32_t>(t * 101u);

        long long total_ns = 0;

        if (op == "insert") {
            total_ns = bench_insert_once(n, p, seed);
        } else if (op == "find") {
            total_ns = bench_find_once(n, p, seed);
        } else if (op == "erase") {
            total_ns = bench_erase_once(n, p, seed);
        } else {
            std::cerr << "Unknown operation: " << op << '\n';
            return 1;
        }

        double avg_ns = static_cast<double>(total_ns) / static_cast<double>(n);

        std::cout << op << ','
                  << n << ','
                  << p << ','
                  << t << ','
                  << total_ns << ','
                  << avg_ns << '\n';
    }

    return 0;
}

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--bench") {
        return run_bench_mode(argc, argv);
    }

    test_default_constructor();
    test_insert_and_find();
    test_count_lower_upper();
    test_range_constructor_and_range_insert();
    test_iterators();
    test_erase_single_and_range();
    test_clear_and_erase_key();
    test_copy_and_move();
    test_template_instantiation();
    test_custom_comparator();
    test_no_default_type();
    test_probability_parameter();
    test_stl_compatibility();

    std::cout << "All tests passed successfully.\n";
    return 0;
}