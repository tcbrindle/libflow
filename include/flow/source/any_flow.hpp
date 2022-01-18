
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_SOURCE_ANY_HPP_INCLUDED
#define FLOW_SOURCE_ANY_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

#include <memory>

namespace flow {

template <typename T>
struct any_flow : flow_base<any_flow<T>> {
private:
    struct iface {
        virtual auto do_next() -> maybe<T> = 0;
        virtual ~iface() = default;
    };

    template <typename F>
    struct impl : iface {
        explicit impl(F flow)
            : flow_(std::move(flow))
        {}

        auto do_next() -> maybe<T> override { return flow_.next(); }
        F flow_;
    };

    std::unique_ptr<iface> ptr_;

public:
    template <typename F,
        std::enable_if_t<!std::is_same_v<remove_cvref_t<F>, any_flow>, int> = 0,
        std::enable_if_t<is_flow<F> && std::is_same_v<item_t<F>, T>, int> = 0>
    any_flow(F flow)
        : ptr_(std::make_unique<impl<F>>(std::move(flow)))
    {}

    auto next() -> maybe<T> { return ptr_->do_next(); }
};


template <typename T>
struct any_flow_ref : flow_base<any_flow_ref<T>> {
private:
    using next_fn_t = auto (void*) -> maybe<T>;

    template <typename F>
    static constexpr next_fn_t* next_fn_impl = +[](void* ptr) {
        return static_cast<F*>(ptr)->next();
    };

    void* ptr_;
    next_fn_t* next_fn_;

public:
    template <typename F,
        std::enable_if_t<!std::is_same_v<std::remove_reference_t<F>, any_flow_ref>, int> = 0,
        std::enable_if_t<is_flow<F> && std::is_same_v<item_t<F>, T>, int> = 0>
    constexpr any_flow_ref(F& flow)
        : ptr_(std::addressof(flow)),
          next_fn_(next_fn_impl<F>)
    {}

    constexpr auto next() -> maybe<T> { return next_fn_(ptr_); }
};

}

#endif
