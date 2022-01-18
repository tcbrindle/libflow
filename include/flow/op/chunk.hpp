
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_OP_CHUNK_HPP_INCLUDED
#define FLOW_OP_CHUNK_HPP_INCLUDED

#include <flow/core/flow_base.hpp>
#include <flow/op/take.hpp>

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

template <typename Flow>
struct chunk_adaptor : flow_base<chunk_adaptor<Flow>>
{
    constexpr chunk_adaptor(Flow&& flow, dist_t size)
        : flow_(std::move(flow)),
          size_(size)
    {}

    constexpr auto next() -> maybe<take_adaptor<subflow_t<Flow>>>
    {
        if (done_) {
            return {};
        }

        auto image = flow_.subflow().take(size_);
        done_ = !(flow_.advance(size_).has_value() && flow_.subflow().next().has_value());
        return image;
    }

private:
    Flow flow_;
    dist_t size_;
    bool done_ = false;
};

}

inline constexpr auto chunk = [](auto&& flowable, dist_t size)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::chunk() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).chunk(size);
};

template <typename D>
constexpr auto flow_base<D>::chunk(dist_t size) &&
{
    static_assert(is_multipass_flow<D>,
                  "chunk() can only be used on multipass flows");
    assert((size > 0) && "Chunk size must be greater than zero");
    return detail::chunk_adaptor<D>(consume(), size);
}

}

#endif
