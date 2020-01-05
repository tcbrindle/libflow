
#include <flow.hpp>

#include "catch.hpp"

#include <iostream>

namespace {

constexpr bool test_group_by() {

    std::array arr{1, 3, -2, -2, 1, 0, 1, 2};

    // An example from the Rust Itertools docs
    {

        bool b = flow::from(arr)
                     .group_by(not flow::pred::negative)
                     .map([](auto f) { return std::move(f).sum(); })
                     .map([](int i) { return i < 0 ? -i : i; })
                     .all(flow::pred::eq(4));

        if (!b) {
            return false;
        }
    }

    // Test round-tripping through group_by() and flatten()
    {
        auto f = flow::from(arr).group_by(!flow::pred::negative).flatten();

        int counter = 0;
        FLOW_FOR(int& i, f) {
            if (&i != arr.data() + counter) {
                return false;
            }
            ++counter;
        }
    }

    // Test not consuming inner groups
    {
        const auto cnt = flow::from(arr).group_by(not flow::pred::negative).count();
        if (cnt != 3) {
        //    return false;
        }
    }

    return true;
}
static_assert(test_group_by());

TEST_CASE("group_by", "[flow.group_by]")
{
    std::vector vec = {1, 3, -2, -2, 1, 0, 1, 2};

    auto groups = flow::from(vec)
        .group_by(not flow::pred::negative)
        .map([](auto f) { return std::move(f).to_vector(); })
        .to_vector();

    for (const auto& g : groups) {
        for (int i : g) {
            std::cout << i << ' ';
        }
        std::cout << "X\n";
    }

    auto sentence = "The quick brown fox jumped over the lazy dog";

    //std::string sentence = "Hi";

    auto words = flow::c_str(sentence)
        .group_by(flow::pred::eq(' '))
        .step_by(2)
        .map([](auto f) { return std::move(f).to_string(); })
        .to_vector();

    std::cout << words.size() << '\n';

    for (auto const& w : words) {
        std::cout << w << " x\n";
    }

    {
        auto sentence = "Hello Darkness My Old Friend";
        flow::c_str(sentence)
            .group_by(flow::pred::eq(' '))
            .step_by(2)
            .map([] (auto f) { return f.next(); })
            .deref()
            .write_to(std::cout, "");
        std::cout << '\n';
    }

    {
        auto alternate = [b = true] (auto&&) mutable {
            return b = !b;
        };

        // Group by, map, group by -- this will break!
        auto f = flow::c_str("Hello World From libFlow")
            .group_by(flow::pred::eq(' '))
            .chunk(2)
            .flatten();

        FLOW_FOR(auto i1, f) {
            std::cout << "* ";
            FLOW_FOR(char c, i1) {
                std::cout << c << ' ';
            }
            std::cout << '\n';
        }
    }

}

}
