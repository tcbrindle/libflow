
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>
#include <iostream>

#ifdef FLOW_HAVE_COROUTINES

struct triple {
    int x, y, z;

    friend std::ostream& operator<<(std::ostream& os, const triple& t)
    {
        os << t.x << ' ' << t.y << ' ' << t.z;
        return os;
    }
};

auto pythagorean_triples() -> flow::async<triple>
{
    FLOW_FOR (int z, flow::ints(1)) {
        FLOW_FOR (int y, flow::ints(1, z)) {
            FLOW_FOR (int x, flow::ints(1, y)) {
                if (x*x + y*y == z*z) {
                    co_yield {x, y, z};
                }
            }
        }
    }
}

int main()
{
    pythagorean_triples().take(10).write_to(std::cout, "\n");
}

#else // !FLOW_HAVE_COROUTINES

int main()
{
    std::cerr << "This example requires coroutines support\n";
}

#endif