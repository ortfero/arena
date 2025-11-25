#pragma once

#include <algorithm>
#include <memory>
#include <utility>

namespace arena {

namespace detail {
template <typename T, typename A> struct vector_buffer {
    using traits = std::allocator_traits<A>;

    // static_assert(traits::propagate_on_container_copy_assignment::value,
    //              "stateful allocator is not supported");
    // static_assert(traits::propagate_on_container_move_assignemnt::value,
    //              "stateful allocator is not supported");

    T *data = nullptr;
    size_t capacity = 0u;
    size_t size = 0u;
    A allr;

    vector_buffer() noexcept = default;

    ~vector_buffer() { cleanup(); }

    vector_buffer(vector_buffer &&other) noexcept
        : data{other.data}, capacity{other.capacity}, size{other.size},
          allr{std::move(other.allr)} {
        other.clear();
    }

    vector_buffer &operator=(vector_buffer &&other) noexcept {
        cleanup();
        allr = std::move(other.allr);
        data = other.data;
        capacity = other.capacity;
        size = other.size;
        other.clear();
        return *this;
    }

    vector_buffer(vector_buffer const &other)
        : data{nullptr}, capacity{0u}, size{0u}, allr{other.allr} {
        if(!other.data)
            return;
        data = traits::allocate(allr, other.capacity);
        capacity = other.capacity;
        size_t n = 0u;
        try {
            for(; n != other.size; ++n)
                traits::construct(allr, data + n, *(other.data + n));
        } catch(...) {
            for(size_t i = 0; i != n; ++i)
                traits::destroy(allr, data + i);
            cleanup();
            clear();
            throw;
        }
        size = n;
    }

    vector_buffer &operator=(vector_buffer const &other) {
        allr = other.allr;
        if(capacity < other.size) {
            cleanup();
            clear();
            data = traits::allocate(allr, other.capacity);
            capacity = other.capacity;
        }
        if(size > other.size) {
            std::copy_n(other.data, other.size, data);
            for(size_t n = other.size; n != size; ++n)
                traits::destroy(allr, data + n);
            size = other.size;
        } else if(size < other.size) {
            std::copy_n(other.data, size, data);
            size_t n = size;
            try {
                for(; n != other.size; ++n)
                    traits::construct(allr, data + n, *(other.data + n));
            } catch(...) {
                size = n;
                throw;
            }
        } else {
            std::copy_n(other.data, size, data);
        }
        return *this;
    }

    void cleanup() {
        if(!data)
            return;
        for(T *item = data; item != data + size; ++item)
            traits::destroy(allr, item);
        traits::deallocate(allr, data, capacity);
    }

    void clear() {
        data = nullptr;
        size = 0u;
        capacity = 0u;
    }

    void reallocate(size_t new_capacity) {
        auto *new_data = traits::allocate(allr, new_capacity);
        size_t n = 0u;
        try {
            for(; n != size; ++n)
                traits::construct(allr, new_data + n, std::move_if_noexcept(*(data + n)));
            ;
        } catch(...) {
            for(size_t i = 0u; i != n; ++i)
                traits::destroy(allr, new_data + i);
            traits::deallocate(allr, new_data, new_capacity);
            throw;
        }
        cleanup();
        data = new_data;
        capacity = new_capacity;
    }

    void destroy(size_t i) { traits::destroy(allr, data + i); }

    template <typename... Args> void construct(size_t i, Args &&...args) {
        traits::construct(allr, data + i, std::forward<Args>(args)...);
    }
};
}   // namespace detail

template <typename T, typename A = std::allocator<T>> class vector {
    detail::vector_buffer<T, A> buffer_;

public:
    using size_type = size_t;
    using value_type = T;

    vector() noexcept = default;
    vector(vector &&other) noexcept = default;
    vector &operator=(vector &&other) noexcept = default;
    vector(vector const &other) = default;
    vector &operator=(vector const &other) = default;

    bool empty() const noexcept { return buffer_.size == 0u; }
    size_type size() const noexcept { return buffer_.size; }
    size_type capacity() const noexcept { return buffer_.capacity; }
    T const &front() const noexcept { return buffer_.data[0]; }
    T const &back() const noexcept { return buffer_.data[buffer_.size - 1]; }

    void pop_back() {
        if(buffer_.size == 0u)
            return;
        --buffer_.size;
        buffer_.destroy(buffer_.size);
    }

    void push_back(T const &item) {
        if(buffer_.size == buffer_.capacity)
            buffer_.reallocate(buffer_.capacity * 3 / 2 + 16);
        buffer_.construct(buffer_.size, item);
        ++buffer_.size;
    }
};   // vector

}   // namespace arena

#ifdef ARENA_ENABLE_TEST

#include <doctest/doctest.h>

TEST_SUITE("vector") {

    SCENARIO("default") {
        auto const target = arena::vector<int>{};
        REQUIRE(target.empty());
        REQUIRE_EQ(target.size(), 0);
        REQUIRE_EQ(target.capacity(), 0);
    }

    SCENARIO("push") {
        auto target = arena::vector<int>{};
        target.push_back(0xCEED);
        REQUIRE(!target.empty());
        REQUIRE_EQ(target.size(), 1);
        REQUIRE_GT(target.capacity(), 0);
        REQUIRE_EQ(target.back(), 0xCEED);
        REQUIRE_EQ(target.front(), 0xCEED);
    }

    SCENARIO("push-push") {
        auto target = arena::vector<int>{};
        target.push_back(0xCEED);
        target.push_back(0xDEAD);
        REQUIRE(!target.empty());
        REQUIRE_EQ(target.size(), 2);
        REQUIRE_GT(target.capacity(), 0);
        REQUIRE_EQ(target.back(), 0xDEAD);
        REQUIRE_EQ(target.front(), 0xCEED);
    }

    SCENARIO("push-pop") {
        auto target = arena::vector<int>{};
        target.push_back(0xCEED);
        target.pop_back();
        REQUIRE(target.empty());
        REQUIRE_EQ(target.size(), 0);
        REQUIRE_GT(target.capacity(), 0);
    }

    SCENARIO("push-push-copy") {
        auto source = arena::vector<int>{};
        source.push_back(0xCEED);
        source.push_back(0xDEAD);
        auto const target = source;
        REQUIRE(!source.empty());
        REQUIRE_EQ(source.size(), 2);
        REQUIRE_GT(source.capacity(), 0);
        REQUIRE_EQ(source.back(), 0xDEAD);
        REQUIRE_EQ(source.front(), 0xCEED);
        REQUIRE(!target.empty());
        REQUIRE_EQ(target.size(), 2);
        REQUIRE_GT(target.capacity(), 0);
        REQUIRE_EQ(target.back(), 0xDEAD);
        REQUIRE_EQ(target.front(), 0xCEED);
    }

    SCENARIO("push-push-move") {
        auto source = arena::vector<int>{};
        source.push_back(0xCEED);
        source.push_back(0xDEAD);
        auto const target = std::move(source);
        REQUIRE(source.empty());
        REQUIRE_EQ(source.size(), 0);
        REQUIRE_EQ(source.capacity(), 0);
        REQUIRE(!target.empty());
        REQUIRE_EQ(target.size(), 2);
        REQUIRE_GT(target.capacity(), 0);
        REQUIRE_EQ(target.back(), 0xDEAD);
        REQUIRE_EQ(target.front(), 0xCEED);
    }
}

#endif   // ARENA_ENABLE_TEST
