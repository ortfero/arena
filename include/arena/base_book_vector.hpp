#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace arena {

class base_book_vector {
public:
    struct order {
        uint64_t id{};
        int64_t price{};
        int64_t amount{};
    };

    bool place(order const &value) {
        if(value.amount == 0 || orders_.contains(value.id))
            return false;
        if(is_bid(value.amount)) {
            auto it = ensure_bid_level(bids_, value.price);
            it->ids.push_back(value.id);
        } else {
            auto it = ensure_ask_level(asks_, value.price);
            it->ids.push_back(value.id);
        }
        orders_.emplace(value.id, value);
        return true;
    }

    bool modify(order const &value) {
        if(value.amount == 0)
            return cancel(value.id);
        if(!orders_.contains(value.id))
            return false;
        if(!cancel(value.id))
            return false;
        return place(value);
    }

    bool cancel(uint64_t id) {
        auto it = orders_.find(id);
        if(it == orders_.end())
            return false;
        auto const current = it->second;
        if(is_bid(current.amount)) {
            auto level_it = find_bid_level(bids_, current.price);
            if(level_it == bids_.end() || !remove_id_from_level(*level_it, id))
                return false;
            if(level_it->ids.empty())
                bids_.erase(level_it);
        } else {
            auto level_it = find_ask_level(asks_, current.price);
            if(level_it == asks_.end() || !remove_id_from_level(*level_it, id))
                return false;
            if(level_it->ids.empty())
                asks_.erase(level_it);
        }

        orders_.erase(it);
        return true;
    }

    std::optional<order> best_bid() const { return best_from_levels(bids_); }
    std::optional<order> best_ask() const { return best_from_levels(asks_); }

private:
    struct price_level {
        int64_t price{};
        std::vector<uint64_t> ids{};
    };

    using level_iterator = std::vector<price_level>::iterator;

    static bool is_bid(int64_t amount) noexcept { return amount > 0; }

    static level_iterator find_bid_insert(std::vector<price_level> &levels, int64_t price) {
        return std::lower_bound(levels.begin(), levels.end(), price,
                                [](price_level const &level, int64_t value) {
                                    return level.price > value;
                                });
    }

    static level_iterator find_ask_insert(std::vector<price_level> &levels, int64_t price) {
        return std::lower_bound(levels.begin(), levels.end(), price,
                                [](price_level const &level, int64_t value) {
                                    return level.price < value;
                                });
    }

    static level_iterator ensure_bid_level(std::vector<price_level> &levels, int64_t price) {
        auto it = find_bid_insert(levels, price);
        if(it == levels.end() || it->price != price)
            it = levels.insert(it, price_level{price});
        return it;
    }

    static level_iterator ensure_ask_level(std::vector<price_level> &levels, int64_t price) {
        auto it = find_ask_insert(levels, price);
        if(it == levels.end() || it->price != price)
            it = levels.insert(it, price_level{price});
        return it;
    }

    static level_iterator find_bid_level(std::vector<price_level> &levels, int64_t price) {
        auto it = find_bid_insert(levels, price);
        if(it != levels.end() && it->price == price)
            return it;
        return levels.end();
    }

    static level_iterator find_ask_level(std::vector<price_level> &levels, int64_t price) {
        auto it = find_ask_insert(levels, price);
        if(it != levels.end() && it->price == price)
            return it;
        return levels.end();
    }

    static bool remove_id_from_level(price_level &level, uint64_t id) {
        for(std::size_t i = 0; i < level.ids.size(); ++i) {
            if(level.ids[i] == id) {
                level.ids[i] = level.ids.back();
                level.ids.pop_back();
                return true;
            }
        }
        return false;
    }

    template <typename Levels> std::optional<order> best_from_levels(Levels const &levels) const {
        if(levels.empty())
            return std::nullopt;
        auto const &front = levels.front();
        if(front.ids.empty())
            return std::nullopt;
        auto const id = front.ids[0];
        auto it = orders_.find(id);
        if(it == orders_.end())
            return std::nullopt;
        return it->second;
    }

    std::unordered_map<uint64_t, order> orders_{};
    std::vector<price_level> bids_{};
    std::vector<price_level> asks_{};
};

}   // namespace arena

#ifdef ARENA_ENABLE_TEST

#include <doctest/doctest.h>

TEST_SUITE("base_book_vector") {

    using book_type = arena::base_book_vector;
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

TEST_SUITE("base_book_vector") {
    SCENARIO("place-1k-cancel") {
        auto rng = std::mt19937_64{12345};
        auto id_dist = std::uniform_int_distribution<uint64_t>{1, 1'000'000};
        auto price_dist = std::uniform_int_distribution<int64_t>{90, 110};
        auto amount_dist = std::uniform_int_distribution<int64_t>{1, 10};

        auto orders = std::vector<arena::base_book_vector::order>{};
        orders.reserve(1000);
        auto used_ids = std::unordered_set<uint64_t>{};
        while(orders.size() < 1000) {
            auto const id = id_dist(rng);
            if(!used_ids.insert(id).second)
                continue;
            orders.push_back(arena::base_book_vector::order{id, price_dist(rng), amount_dist(rng)});
        }

        auto book = arena::base_book_vector{};
        auto bench = ankerl::nanobench::Bench{};
        bench.title("base_book_vector");
        bench.run("place 1k orders", [&] { 
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
