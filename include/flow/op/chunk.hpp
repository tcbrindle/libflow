
#ifndef FLOW_OP_CHUNK_HPP_INCLUDED
#define FLOW_OP_CHUNK_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

struct chunk_counter {

    explicit constexpr chunk_counter(dist_t size)
        : size_(size)
    {}

    template <typename T>
    constexpr auto operator()(T&& /*unused*/) -> bool
    {
        if (++counter_ < size_) {
            return last_;
        }
        counter_ = 0;
        return (last_ = !last_);
    }

private:
    const dist_t size_;
    dist_t counter_ = -1; // Begin off-track
    bool last_ = true;
};

}

template <typename Derived>
constexpr auto flow_base<Derived>::chunk(dist_t size) &&
{
    assert((size > 0) && "Chunk size must be greater than zero");
    return consume().group_by(detail::chunk_counter{size});
}

}

#endif
