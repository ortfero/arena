#pragma once

#include <arena/arena.hpp>

TEST_SUITE("arena") {

    SCENARIO("vector:default") {
        auto const target = arena::vector<int>{};
        REQUIRE(target.empty());
        REQUIRE_EQ(target.size(), 0);
        REQUIRE_EQ(target.capacity(), 0);
    }

    SCENARIO("vector:push") {
        auto target = arena::vector<int>{};
        target.push_back(0xCEED);
        REQUIRE(!target.empty());
        REQUIRE_EQ(target.size(), 1);
        REQUIRE_GT(target.capacity(), 0);
        REQUIRE_EQ(target.back(), 0xCEED);
        REQUIRE_EQ(target.front(), 0xCEED);
    }

    SCENARIO("vector:push-push") {
        auto target = arena::vector<int>{};
        target.push_back(0xCEED);
        target.push_back(0xDEAD);
        REQUIRE(!target.empty());
        REQUIRE_EQ(target.size(), 2);
        REQUIRE_GT(target.capacity(), 0);
        REQUIRE_EQ(target.back(), 0xDEAD);
        REQUIRE_EQ(target.front(), 0xCEED);
    }

    SCENARIO("vector:push-pop") {
        auto target = arena::vector<int>{};
        target.push_back(0xCEED);
        target.pop_back();
        REQUIRE(target.empty());
        REQUIRE_EQ(target.size(), 0);
        REQUIRE_GT(target.capacity(), 0);
    }

    SCENARIO("vector:push-push-copy") {
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

    SCENARIO("vector:push-push-move") {
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
