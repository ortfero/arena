#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace arena {

class base_book_map {
public:
    struct order {
        uint64_t id{};
        int64_t price{};
        int64_t amount{};
    };
    
private:
    using bid_book = std::map<int64_t, uint64_t, std::greater<int64_t>>;
    using ask_book = std::map<int64_t, uint64_t, std::less<int64_t>>;

    std::unordered_map<uint64_t, order> orders_{};
    bid_book bids_{};
    ask_book asks_{};

public:


    bool place(order const &value) {
        if(value.amount == 0 || orders_.contains(value.id))
            return false;
        orders_.emplace(value.id, value);
        if(!add_to_book(value)) {
            orders_.erase(value.id);
            return false;
        }
        return true;
    }

    bool modify(order const &value) {
        if(value.amount == 0)
            return cancel(value.id);

        auto it = orders_.find(value.id);
        if(it == orders_.end())
            return false;

        auto const previous = it->second;
        if(is_bid(previous.amount) == is_bid(value.amount) && previous.price == value.price) {
            it->second = value;
            return true;
        }

        if(!remove_from_book(previous))
            return false;
        it->second = value;
        if(!add_to_book(value)) {
            it->second = previous;
            add_to_book(previous);
            return false;
        }
        return true;
    }

    bool cancel(uint64_t id) {
        auto it = orders_.find(id);
        if(it == orders_.end())
            return false;
        auto const value = it->second;
        if(!remove_from_book(value))
            return false;
        orders_.erase(it);
        return true;
    }

    std::optional<order> best_bid() const { return best_from_book(bids_); }
    std::optional<order> best_ask() const { return best_from_book(asks_); }

private:


    static bool is_bid(int64_t amount) noexcept { return amount > 0; }

    bool add_to_book(order const &value) {
        if(is_bid(value.amount)) {
            bids_.emplace(value.price, value.id);
            return true;
        }
        asks_.emplace(value.price, value.id);
        return true;
    }

    bool remove_from_book(order const &value) {
        if(is_bid(value.amount)) {
            auto const range = bids_.equal_range(value.price);
            for(auto it = range.first; it != range.second; ++it) {
                if(it->second == value.id) {
                    bids_.erase(it);
                    return true;
                }
            }
            return false;
        }
        auto const range = asks_.equal_range(value.price);
        for(auto it = range.first; it != range.second; ++it) {
            if(it->second == value.id) {
                asks_.erase(it);
                return true;
            }
        }
        return false;
    }

    template <typename Book> std::optional<order> best_from_book(Book const &book) const {
        if(book.empty())
            return std::nullopt;
        auto it = orders_.find(book.begin()->second);
        if(it == orders_.end())
            return std::nullopt;
        return it->second;
    }
};

}   // namespace arena

#ifdef ARENA_ENABLE_TEST

#include <doctest/doctest.h>

TEST_SUITE("base_book_map") {

    using book_type = arena::base_book_map;
    using order = typename book_type::order;

    TEST_CASE("place_and_best_bid") {
        auto book = book_type{};
        REQUIRE(book.place(order{1u, 100, 5}));
        auto const best_bid = book.best_bid();
        REQUIRE(best_bid.has_value());
        CHECK_EQ(best_bid->id, 1u);
        CHECK_EQ(best_bid->price, 100);
        CHECK_EQ(best_bid->amount, 5);
        CHECK_FALSE(book.best_ask().has_value());
    }

    TEST_CASE("place_and_best_ask") {
        auto book = book_type{};
        REQUIRE(book.place(order{2u, 101, -3}));
        auto const best_ask = book.best_ask();
        REQUIRE(best_ask.has_value());
        CHECK_EQ(best_ask->id, 2u);
        CHECK_EQ(best_ask->price, 101);
        CHECK_EQ(best_ask->amount, -3);
        CHECK_FALSE(book.best_bid().has_value());
    }

    TEST_CASE("best_prices_follow_ordering") {
        auto book = book_type{};
        REQUIRE(book.place(order{1u, 100, 5}));
        REQUIRE(book.place(order{2u, 99, 7}));
        REQUIRE(book.place(order{3u, 101, 1}));

        auto const best_bid = book.best_bid();
        REQUIRE(best_bid.has_value());
        CHECK_EQ(best_bid->id, 3u);
        CHECK_EQ(best_bid->price, 101);
        CHECK_EQ(best_bid->amount, 1);

        REQUIRE(book.place(order{4u, 102, -2}));
        REQUIRE(book.place(order{5u, 98, -4}));
        auto const best_ask = book.best_ask();
        REQUIRE(best_ask.has_value());
        CHECK_EQ(best_ask->id, 5u);
        CHECK_EQ(best_ask->price, 98);
        CHECK_EQ(best_ask->amount, -4);
    }

    TEST_CASE("modify_moves_order_between_levels") {
        auto book = book_type{};
        REQUIRE(book.place(order{1u, 100, 5}));
        REQUIRE(book.place(order{2u, 99, 7}));

        REQUIRE(book.modify(order{2u, 101, 7}));
        auto const best_bid = book.best_bid();
        REQUIRE(best_bid.has_value());
        CHECK_EQ(best_bid->id, 2u);
        CHECK_EQ(best_bid->price, 101);

        REQUIRE(book.modify(order{2u, 90, -6}));
        auto const best_ask = book.best_ask();
        REQUIRE(best_ask.has_value());
        CHECK_EQ(best_ask->id, 2u);
        CHECK_EQ(best_ask->price, 90);
        CHECK_EQ(best_ask->amount, -6);
    }

    TEST_CASE("cancel_removes_orders") {
        auto book = book_type{};
        REQUIRE(book.place(order{1u, 100, 5}));
        REQUIRE(book.place(order{2u, 101, -3}));
        REQUIRE(book.cancel(1u));
        CHECK_FALSE(book.best_bid().has_value());
        CHECK(book.cancel(2u));
        CHECK_FALSE(book.best_ask().has_value());
    }
}

#endif   // ARENA_ENABLE_TEST

#ifdef ARENA_ENABLE_BENCH

#include <doctest/doctest.h>
#include <nanobench.h>

TEST_SUITE("base_book_map") {
    SCENARIO("place-1k-cancel") {
        auto rng = std::mt19937_64{12345};
        auto id_dist = std::uniform_int_distribution<uint64_t>{1, 1'000'000};
        auto price_dist = std::uniform_int_distribution<int64_t>{90, 110};
        auto amount_dist = std::uniform_int_distribution<int64_t>{1, 10};

        auto orders = std::vector<arena::base_book_map::order>{};
        orders.reserve(1000);
        auto used_ids = std::unordered_set<uint64_t>{};
        while(orders.size() < 1000) {
            auto const id = id_dist(rng);
            if(!used_ids.insert(id).second)
                continue;
            orders.push_back(arena::base_book_map::order{id, price_dist(rng), amount_dist(rng)});
        }

        auto book = arena::base_book_map{};
        auto bench = ankerl::nanobench::Bench{};
        bench.title("base_book_map");
        bench.minEpochIterations(3712).run("place 1k orders", [&] {
            for(auto const &order : orders)
                ankerl::nanobench::doNotOptimizeAway(book.place(order));
            for(auto const &order : orders)
                ankerl::nanobench::doNotOptimizeAway(book.cancel(order.id));
            
        });
        // bench.run("cancel 1k orders", [&] {
        //     for(auto const &order : orders)
        //         ankerl::nanobench::doNotOptimizeAway(book.cancel(order.id));
        // });
    }
}


#endif // ARENA_ENABLE_BENCH
