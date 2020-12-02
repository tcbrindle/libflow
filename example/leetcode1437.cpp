
#include <flow.hpp>

// Given an array nums of 0s and 1s and an integer k, return True if all 1's are
// at least k places away from each other, otherwise return False.
// https://leetcode.com/problems/check-if-all-1s-are-at-least-length-k-places-away/

constexpr auto k_length_apart = [](const auto& arr, int k)
{
    return flow::split(arr, 1).map(flow::count).all(flow::pred::geq(k));
};

int main()
{
    {
        constexpr auto nums = std::array{1, 0, 0, 0, 1, 0, 0, 0, 1};
        constexpr auto k = 2;
        static_assert(k_length_apart(nums, k));
    }

    {
        constexpr auto nums = std::array{1, 0, 0, 1, 0, 1};
        constexpr auto k = 2;
        static_assert(not k_length_apart(nums, k));
    }

    {
        constexpr auto nums = std::array{1, 1, 1, 1, 1};
        constexpr auto k = 0;
        static_assert(k_length_apart(nums, k));
    }

    {
        constexpr auto nums = std::array{0, 1, 0, 1};
        constexpr auto k = 1;
        static_assert(k_length_apart(nums, k));
    }
}