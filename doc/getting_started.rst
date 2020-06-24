
Getting Started
===============

Getting *libflow*
-----------------

The easiest way to get started with *libflow* is to download the latest,
automatically-generated header, and then add it to your C++ project like
any other header.

Once you have obtained the header and placed it in your project, using
*libflow* is as simple as `#include`-ing the `flow.hpp` header::

    #include <flow.hpp>

    // Let's start using libflow!

Your first flow
-----------------

*libflow* is based around the idea of data pipelines, otherwise known as
**flows**. A flow begins with a **source**, may pass through optional
**adaptors**, before arriving at an **output**.

For example, here's how we can use *libflow* to compute the sum of the squares of
the even numbers between 1 and 10 (at compile time)::

    constexpr auto sum =
        flow::ints(1)                           // source
            .take(10)                           // adaptor
            .filter(flow::pred::even)           // adaptor
            .map([](int i) { return i * i; })   // adaptor
            .sum();                             // output

    static_assert(sum == 220);


