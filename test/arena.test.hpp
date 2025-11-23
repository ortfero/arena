#pragma once

#include <arena/arena.hpp>

TEST_SUITE("arena") {
    SCENARIO("default constructor") {
        auto const target = arena::component{};
        REQUIRE_EQ(target.x, 0);
        REQUIRE_EQ(target.y, 0);
        REQUIRE_EQ(target.z, 0);
    };
}
