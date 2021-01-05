
#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

using namespace std::string_view_literals;

constexpr bool test_split()
{
    {
        auto f = flow::c_str("Hello world this   is a test").split(' ');

        if (f.subflow().count() != 6) {
            return false;
        }

        bool b =
            f.next()->equal("Hello"sv) &&
            f.next()->equal("world"sv) &&
            f.next()->equal("this"sv) &&
            f.next()->equal("is"sv) &&
            f.next()->equal("a"sv) &&
            f.next()->equal("test"sv) &&
            !f.next().has_value();

        if (!b) {
            return false;
        }
    }

    {
        auto f = flow::c_str("   Hello world another   test   ").split(' ');

        if (f.subflow().count() != 4) {
            return false;
        }

        bool b =
            f.next()->equal("Hello"sv) &&
                f.next()->equal("world"sv) &&
                f.next()->equal("another"sv) &&
                f.next()->equal("test"sv) &&
                !f.next().has_value();

        if (!b) {
            return false;
        }
    }

    {
        auto f = flow::split(L"hello____again"sv, L'_');

        if (f.subflow().count() != 2) {
            return false;
        }

        bool b =
            f.next()->equal(L"hello"sv) &&
            f.next()->equal(L"again"sv) &&
            !f.next().has_value();

        if (!b) {
            return false;
        }
    }

    return true;
}
#if !COMPILER_IS_MSVC
static_assert(test_split());
#endif

TEST_CASE("Split with delimiter", "[flow.split]")
{
    REQUIRE(test_split());
}

}

