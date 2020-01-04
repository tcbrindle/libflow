
#include <flow.hpp>

#include <iostream>
#include <forward_list>
#include <map>
#include <sstream>

#include "catch.hpp"

namespace {
#ifndef _MSC_VER
constexpr bool test_std_array()
{
    // lvalue
    {
        std::array arr = {1, 2, 3, 4, 5};

        int counter = 0;
        FLOW_FOR(int& i, flow::from(arr)) {
            if (i != arr[counter++]) {
                return false;
            }
        }
    }

    // rvalue
    {
        std::array arr = {1, 2, 3, 4, 5};

        int counter = 0;
        FLOW_FOR(int i, flow::from(std::array{1, 2, 3, 4, 5})) {
            if (i != arr[counter++]) {
                return false;
            }
        }
    }
    return true;
}
static_assert(test_std_array());

constexpr bool test_raw_array()
{
    // lvalue
    {
        int arr[] = {1, 2, 3, 4, 5};

        int counter = 0;
        FLOW_FOR(int& i, flow::from(arr)) {
                    if (i != arr[counter++]) {
                        return false;
                    }
                }
    }

    // rvalue
    {
        using arr_t = int[5];
        arr_t arr = {1, 2, 3, 4, 5};

        int counter = 0;
        FLOW_FOR(int i, flow::from(arr_t{1, 2, 3, 4, 5})) {
            if (i != arr[counter++]) {
                return false;
            }
        }
    }
    return true;
}
static_assert(test_raw_array());

constexpr bool test_iter_sentinel_pair()
{
    std::array arr{1, 2, 3, 4, 5};

    int counter = 0;
    FLOW_FOR(int i, flow::from(arr.begin(), arr.end())) {
                if (i != arr[counter++]) {
                    return false;
                }
            }
    return true;
}
static_assert(test_iter_sentinel_pair());
#endif

TEST_CASE("flow::from() lvalue random-access range", "[flow.from]")
{
    std::vector<int> vec{1, 2, 3, 4, 5};

    FLOW_FOR(auto& i, flow::from(vec)) {
        i = 0;
    }

    for (int i : vec) {
        REQUIRE(i == 0);
    }
}

TEST_CASE("flow::from() rvalue random-access range", "[flow.from]")
{
    std::array test{1, 2, 3, 4, 5};

    int counter = 0;
    FLOW_FOR(auto const& i, flow::from(std::vector{1, 2, 3, 4, 5})) {
        REQUIRE(i == test[counter++]);
    }
}

TEST_CASE("flow::from lvalue forward range", "[flow.from]")
{
   std::forward_list<int> list{1, 2, 3, 4, 5};

    FLOW_FOR(int& i,  flow::from(list)) {
        i = 0;
    }

    for (int i : list) {
        REQUIRE(i == 0);
    }
}

TEST_CASE("flow::from rvalue forward range", "[flow.from]")
{
    std::array test{1, 2, 3, 4, 5};

    int counter = 0;
    FLOW_FOR(auto const& i, flow::from(std::forward_list{1, 2, 3, 4, 5})) {
        REQUIRE(i == test[counter++]);
    }
}

template <typename T, typename Stream>
struct istream_range {
    explicit istream_range(Stream& stream)
        : first_(stream)
    {}

    auto begin() const { return first_; }
    auto end() const { return last_; }

    std::istream_iterator<T> first_;
    std::istream_iterator<T> last_{};
};


TEST_CASE("flow::from rvalue input range", "[flow.from]")
{
    std::istringstream oss{"1 2 3 4 5"};
    std::array<int, 5> test{1, 2, 3, 4, 5};

    int counter = 0;
    FLOW_FOR(int i, flow::from(istream_range<int, std::istringstream>(oss))) {
        REQUIRE(i == test[counter++]);
    }
}

TEST_CASE("flow::from std::map", "[flow.from]")
{
    std::map<std::string, int> map{
        { "1", 1},
        { "2", 2},
        { "3", 3}
    };

    int counter = 1;
    // FIXME: work out the macro magic so we can use structured bindings with FOR
    FLOW_FOR(auto const& pair, flow::from(map)) {
        auto const& [str, i] = pair;
        REQUIRE(str == std::to_string(counter));
        REQUIRE(i == counter);
        ++counter;
    }
}

}
