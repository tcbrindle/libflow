
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_SOURCE_ISTREAMBUF_HPP_INCLUDED
#define FLOW_SOURCE_ISTREAMBUF_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename CharT, typename Traits>
struct istreambuf_flow : flow_base<istreambuf_flow<CharT, Traits>> {

    using streambuf_type = std::basic_streambuf<CharT, Traits>;

    explicit istreambuf_flow(streambuf_type* buf)
        : buf_(buf)
    {}

    istreambuf_flow(istreambuf_flow&&) = default;
    istreambuf_flow& operator=(istreambuf_flow&&) = default;

    auto next() -> maybe<CharT>
    {
        if (!buf_) {
            return {};
        }

        auto c = buf_->sbumpc();

        if (c == Traits::eof()) {
            buf_ = nullptr;
            return {};
        }

        return Traits::to_char_type(c);
    }

private:
    streambuf_type* buf_ = nullptr;
};

struct from_istreambuf_fn {

    template <typename CharT, typename Traits>
    auto operator()(std::basic_streambuf<CharT, Traits>* buf) const
    {
        return istreambuf_flow<CharT, Traits>(buf);
    }

    template <typename CharT, typename Traits>
    auto operator()(std::basic_istream<CharT, Traits>& stream) const
    {
        return istreambuf_flow<CharT, Traits>(stream.rdbuf());
    }
};

} // namespace detail

inline constexpr auto from_istreambuf = detail::from_istreambuf_fn{};

} // namespace flow

#endif
