
#ifndef FLOW_SOURCE_FROM_HPP_INCLUDED
#define FLOW_SOURCE_FROM_HPP_INCLUDED

#include <array>
#include <iterator>

namespace flow {

namespace detail {

template <typename R>
using iterator_t = decltype(std::begin(std::declval<R&>()));

template <typename R>
using sentinel_t = decltype(std::end(std::declval<R&>()));

template <typename R>
using iter_reference_t = decltype(*std::declval<iterator_t<R>&>());

template <typename R>
using iter_value_t = remove_cvref_t<iter_reference_t<R>>;

template <typename R>
using iter_category_t =
    typename std::iterator_traits<iterator_t<R>>::iterator_category;

template <typename, typename = void>
inline constexpr bool is_stl_range = false;

template <typename R>
inline constexpr bool is_stl_range<
    R, std::void_t<iterator_t<R>, sentinel_t<R>, iter_category_t<R>>> = true;

template <typename R>
inline constexpr bool is_fwd_stl_range =
    std::is_base_of_v<std::forward_iterator_tag, iter_category_t<R>>;

template <typename R>
inline constexpr bool is_random_access_stl_range =
    std::is_base_of_v<std::random_access_iterator_tag, iter_category_t<R>>;

template <typename R>
struct range_ref {

    constexpr range_ref(R& rng) : ptr_(std::addressof(rng)) {}

    constexpr auto begin() const { return std::begin(*ptr_); }
    constexpr auto end() const { return std::end(*ptr_); }

private:
    R* ptr_;
};

template <typename I, typename S>
struct iterator_range {

    constexpr auto begin() const { return first; }
    constexpr auto end() const { return last; }

    I first;
    S last;
};

template <typename R>
struct stl_input_range_adaptor : flow_base<stl_input_range_adaptor<R>> {
private:
    R rng_;
    iterator_t<R> cur_ = std::begin(rng_);

public:
    constexpr explicit stl_input_range_adaptor(R&& range)
        : rng_(std::move(range))
    {}

    // We are move-only
    constexpr stl_input_range_adaptor(stl_input_range_adaptor&&) = default;
    constexpr stl_input_range_adaptor&
    operator=(stl_input_range_adaptor&&) = default;

    constexpr auto next() -> maybe<iter_value_t<R>>
    {
        if (cur_ != std::end(rng_)) {
            auto val = *cur_;
            ++cur_;
            return {std::move(val)};
        }
        return {};
    }
};

template <typename R>
struct stl_fwd_range_adaptor : flow_base<stl_fwd_range_adaptor<R>> {
public:
    constexpr explicit stl_fwd_range_adaptor(R&& range) : rng_(std::move(range))
    {}

    constexpr auto next() -> maybe<detail::iter_reference_t<R>>
    {
        if (cur_ != std::end(rng_)) {
            return {*cur_++};
        }
        return {};
    }

private:
    R rng_;
    iterator_t<R> cur_ = std::begin(rng_);
};

template <typename R>
struct stl_ra_range_adaptor : flow_base<stl_ra_range_adaptor<R>> {

    template <typename RR = R, std::enable_if_t<!std::is_array_v<RR>, int> = 0>
    constexpr explicit stl_ra_range_adaptor(R&& rng) : rng_(FLOW_FWD(rng))
    {}

    // Why would anyone ever try to use a raw language array with flow::from?
    // I don't know, but let's try to support them in their foolish quest anyway
    template <typename T, std::size_t N>
    constexpr explicit stl_ra_range_adaptor(T(&&arr)[N])
    {
        for (std::size_t i = 0; i < N; i++) {
            rng_[i] = std::move(arr)[i];
        }
    }

    constexpr auto next() -> maybe<iter_reference_t<R>>
    {
        if (idx_ < std::end(rng_) - std::begin(rng_)) {
            return {std::begin(rng_)[idx_++]};
        }
        return {};
    }

    constexpr auto advance(dist_t dist)
    {
        assert(dist > 0);
        idx_ += dist - 1;
        return next();
    }

private:
    R rng_{};
    dist_t idx_ = 0;
};

struct from_fn {
    template <typename R>
    constexpr auto operator()(R&& range) const
    {
        static_assert(
            is_stl_range<R>,
            "Argument to flow::from() does not look like an STL range");

        using rng_t =
            std::conditional_t<std::is_lvalue_reference_v<R>,
                               range_ref<std::remove_reference_t<R>>, R>;

        if constexpr (is_random_access_stl_range<R>) {
            return stl_ra_range_adaptor<rng_t>{FLOW_FWD(range)};
        } else if constexpr (is_fwd_stl_range<R>) {
            return stl_fwd_range_adaptor<rng_t>{FLOW_FWD(range)};
        } else {
            static_assert(
                !std::is_lvalue_reference_v<R>,
                "Input ranges must be passed as rvalues -- use std::move()");
            return stl_input_range_adaptor<R>{FLOW_FWD(range)};
        }
    }

    template <typename I, typename S>
    constexpr auto operator()(I iterator, S sentinel) const
    {
        return from_fn{}(iterator_range<I, S>{std::move(iterator), std::move(sentinel)});
    }
};

} // namespace detail

inline constexpr detail::from_fn from{};

} // namespace flow

#endif
