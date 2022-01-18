
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_SOURCE_FROM_ISTREAM_HPP_INCLUDED
#define FLOW_SOURCE_FROM_ISTREAM_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename CharT, typename Traits>
struct istream_ref {
    std::basic_istream<CharT, Traits>* ptr_;

    template <typename T>
    friend constexpr auto& operator>>(istream_ref& self, T& item)
    {
        return *self >> item;
    }
};

template <typename>
inline constexpr bool is_istream = false;

template <typename CharT, typename Traits>
inline constexpr bool is_istream<std::basic_istream<CharT, Traits>> = true;

template <typename T, typename CharT, typename Traits>
struct istream_flow : flow_base<istream_flow<T, CharT, Traits>> {
private:
    using istream_type = std::basic_istream<CharT, Traits>;
    istream_type* is_;
    T item_ = T();

public:
    explicit istream_flow(std::basic_istream<CharT, Traits>& is)
        : is_(std::addressof(is))
    {}

    // Move-only
    istream_flow(istream_flow&&) noexcept = default;
    istream_flow& operator=(istream_flow&&) noexcept = default;

    auto next() -> maybe<T>
    {
        if (!(*is_ >> item_)) { return {}; }
        return {item_};
    }
};

} // namespace detail

template <typename T, typename CharT, typename Traits>
auto from_istream(std::basic_istream<CharT, Traits>& is)
{
    return detail::istream_flow<T, CharT, Traits>(is);
}

}

#endif
