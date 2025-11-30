#pragma once

#include <optional>
#include <vector>

namespace arena {
// Architecture task. 
// Write a LoadBalancer class with two members: AddResource and GetResource. 

// GetResource should work in a round-robin way: 
// - after calling AddResource 3 times with values 1, 2, 3,
// GetResource called 4 times in a row must return 1, 2, 3, 1:
//

struct RoundRobinPolicy {
private:
    size_t index_ = 0u;

public:
    size_t next_index(size_t size) noexcept {
        auto const last = index_++;
        if(index_ == size)
            index_ = 0;
        return last;
    }
};

template <typename T, class Policy = RoundRobinPolicy> class LoadBalancer {
    std::vector<T> data_;
    Policy policy_;

public:
    LoadBalancer() = default;

    void AddResource(T const &item) { data_.push_back(item); }

    std::optional<T> GetResource() {
        if(data_.empty())
            return std::nullopt;
        return {data_[policy_.next_index(data_.size())]};
    }
};   // LoadBalancer
} // arena

#ifdef ARENA_ENABLE_TEST

TEST_SUITE("probe") {

  SCENARIO("some") {
    auto target = arena::LoadBalancer<int>{};
    target.AddResource(1);
    target.AddResource(2);
    target.AddResource(3);
    REQUIRE_EQ(target.GetResource().value(), 1);
    REQUIRE_EQ(target.GetResource().value(), 2);
    REQUIRE_EQ(target.GetResource().value(), 3);
    REQUIRE_EQ(target.GetResource().value(), 1);
  }

  SCENARIO("get-from-empty") {
    auto target = arena::LoadBalancer<int>{};
    auto r = target.GetResource();
    REQUIRE(!r.has_value());
  }
  
}

#endif
