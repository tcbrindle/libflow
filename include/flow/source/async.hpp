
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_SOURCE_ASYNC_HPP_INCLUDED
#define FLOW_SOURCE_ASYNC_HPP_INCLUDED

#include <flow/core/macros.hpp>

#ifdef FLOW_HAVE_COROUTINES

#include <flow/core/flow_base.hpp>

#ifdef FLOW_HAVE_CPP20_COROUTINES
#include <coroutine>
#else
#include <experimental/coroutine>
#endif

namespace flow {

namespace detail {

#ifdef FLOW_HAVE_CPP20_COROUTINES
namespace coro_ns = std;
#else
namespace coro_ns = std::experimental;
#endif

}

template <typename T>
struct async : flow_base<async<T>>
{
    struct promise_type;

    using handle_type = detail::coro_ns::coroutine_handle<promise_type>;

    struct promise_type {
    private:
        maybe<T> value{};

    public:
        auto initial_suspend() {
            return detail::coro_ns::suspend_always{};
        };

        auto final_suspend() noexcept {
            return detail::coro_ns::suspend_always{};
        }

        auto yield_value(std::remove_reference_t<T>& val)
        {
            value = maybe<T>(val);
            return detail::coro_ns::suspend_always{};
        }

        auto yield_value(remove_cvref_t<T>&& val)
        {
            value = maybe<T>(std::move(val));
            return detail::coro_ns::suspend_always{};
        }

        auto extract_value() -> maybe<T>
        {
            auto ret = std::move(value);
            value.reset();
            return ret;
        }

        auto get_return_object()
        {
            return async{handle_type::from_promise(*this)};
        }

        auto unhandled_exception()
        {
            std::terminate();
        }

        auto return_void()
        {
            return detail::coro_ns::suspend_never{};
        }
    };

    auto next() -> maybe<T>
    {
        if (!coro.done()) {
            coro.resume();
            return coro.promise().extract_value();
        }
        return {};
    }

    async(async&& other) noexcept
        : coro(std::move(other).coro)
    {
        other.coro = nullptr;
    }

    async& operator=(async&& other) noexcept
    {
        std::swap(coro, other.coro);
    }

    ~async()
    {
        if (coro) {
            coro.destroy();
        }
    }

private:
    explicit async(handle_type&& handle)
        : coro(std::move(handle))
    {}

    handle_type coro;
};

} // namespace flow

#endif // FLOW_HAVE_COROUTINES

#endif
