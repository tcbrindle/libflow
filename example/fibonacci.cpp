
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flow.hpp>

#include <iostream>

struct fibonacci : flow::flow_base<fibonacci> {

    explicit fibonacci(int a = 1, int b = 1) : a(a), b(b) {}

    flow::maybe<int> next()
    {
        int temp = a;
        a = b;
        b = a + temp;
        return temp;
    }

private:
    int a;
    int b;
};

int main()
{
    std::cout << "The first ten Fibonacci numbers are:\n";
    fibonacci{}.take(10).write_to(std::cout);
}