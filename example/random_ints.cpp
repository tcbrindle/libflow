
// An example of writing a custom flow, in this case one that generates
// an endless sequence of random ints within a given interval

#include <flow.hpp>

#include <iostream>
#include <random>

struct random_ints : flow::flow_base<random_ints> {

    random_ints(int from, int to)
        : dist(from, to)
    {}

    auto next() -> flow::maybe<int>
    {
        return {dist(gen)};
    }

private:
    std::mt19937 gen{std::random_device{}()};
    std::uniform_int_distribution<int> dist;
};

int main()
{
    random_ints(0, 100)
        .take(10)
        .write_to(std::cout, "\n");
}