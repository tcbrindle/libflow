
#include <flow.hpp>

#include "catch.hpp"
#include "macros.hpp"

namespace {

constexpr bool test_c_str()
{
    if (flow::c_str{"Hello"}.count() != 5) {
        return false;
    }

    auto const& str = L"World";
    int i = 0;
    FLOW_FOR(wchar_t c, flow::c_str(str)) {
        if (c != str[i++]) {
            return false;
        }
    }

    if (flow::c_str("").count() != 0) {
        return false;
    }

    if (flow::c_str(u"embedded\0\0\0\0nulls").count() !=
        (flow::dist_t) std::char_traits<char16_t>::length(u"embedded")) {
        return false;
    }

    return true;
}
#if !COMPILER_IS_MSVC
static_assert(test_c_str());
#endif

TEST_CASE("flow::c_str", "[flow.c_str]")
{
    REQUIRE(test_c_str());

    std::string str = flow::c_str("abcdef\0\0g").collect();
    REQUIRE(str == "abcdef");
}


}
