
#ifndef FLOW_SOURCE_FROM_HPP_INCLUDED
#define FLOW_SOURCE_FROM_HPP_INCLUDED

#include <cassert>
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
using iter_value_t = typename std::iterator_traits<iterator_t<R>>::value_type;

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
inline constexpr bool is_bidir_stl_range =
    std::is_base_of_v<std::bidirectional_iterator_tag, iter_category_t<R>>;

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

template <typename>
inline constexpr bool is_range_ref = false;

template <typename R>
inline constexpr bool is_range_ref<range_ref<R>> = true;

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

    constexpr auto to_range() &&
    {
        return std::move(rng_);
    }

    template <typename C>
    constexpr auto to() &&
    {
        if constexpr (std::is_same_v<C, R>) {
            return std::move(rng_);
        } else {
            return C(cur_, std::end(rng_));
        }
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

    constexpr auto to_range() &&
    {
        return std::move(rng_);
    }

    template <typename C>
    constexpr auto to() &&
    {
        if constexpr (std::is_same_v<C, R>) {
            return std::move(rng_);
        } else {
            return C(cur_, std::end(rng_));
        }
    }

    constexpr auto subflow() &
    {
        if  constexpr (is_range_ref<R>) {
            return *this;
        } else {
            auto s = stl_fwd_range_adaptor<range_ref<R>>(rng_);
            s.cur_ = cur_;
            return s;
        }
    }

private:
    template <typename>
    friend struct stl_fwd_range_adaptor;

    R rng_;
    iterator_t<R> cur_ = std::begin(rng_);
};

template <typename R>
struct stl_bidir_range_adaptor : flow_base<stl_bidir_range_adaptor<R>> {

    constexpr explicit stl_bidir_range_adaptor(R&& rng)
        : rng_(std::move(rng))
    {}

    constexpr auto next() -> maybe<iter_reference_t<R>>
    {
        if (first_ != last_) {
            return {*first_++};
        }
        return {};
    }

    constexpr auto next_back() -> maybe<iter_reference_t<R>>
    {
        if (first_ != last_) {
            return {*--last_};
        }
        return {};
    }

    template <typename C>
    constexpr auto to() &&
    {
        return C(first_, last_);
    }

    constexpr auto subflow() &
    {
        if  constexpr (is_range_ref<R>) {
            return *this;
        } else {
            auto s = stl_bidir_range_adaptor<range_ref<R>>(rng_);
            s.first_ = first_;
            s.last_ = last_;
            return s;
        }
    }

private:
    R rng_;
    iterator_t<R> first_ = std::begin(rng_);
    iterator_t<R> last_ = std::end(rng_);
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
        : idx_back_(N)
    {
        for (std::size_t i = 0; i < N; i++) {
            rng_[i] = std::move(arr)[i];
        }
    }

    constexpr auto next() -> maybe<iter_reference_t<R>>
    {
        if (idx_ < idx_back_) {
            return {std::begin(rng_)[idx_++]};
        }
        return {};
    }

    constexpr auto next_back() -> maybe<iter_reference_t<R>>
    {
        if (idx_back_ > idx_ ) {
            return {std::begin(rng_)[--idx_back_]};
        }
        return {};
    }

    constexpr auto advance(dist_t dist)
    {
        assert(dist > 0);
        idx_ += dist - 1;
        return next();
    }

    constexpr auto to_range() &&
    {
        return std::move(rng_);
    }

    template <typename C>
    constexpr auto to() &&
    {
        if constexpr (std::is_same_v<C, R>) {
            return std::move(rng_);
        } else {
            return C(std::begin(rng_) + idx_, std::end(rng_));
        }
    }

    constexpr auto subflow() &
    {
        if constexpr (is_range_ref<R>) {
            return *this; // just copy
        } else {
            auto f = stl_ra_range_adaptor<range_ref<R>>(rng_);
            f.idx_ = idx_;
            return f;
        }
    }

    [[nodiscard]] constexpr auto size() const -> dist_t
    {
        return idx_back_ - idx_;
    }

private:
    template <typename>
    friend struct stl_ra_range_adaptor;

    R rng_{};
    dist_t idx_ = 0;
    dist_t idx_back_ = std::distance(std::begin(rng_), std::end(rng_));
};

template <typename R, typename = std::enable_if_t<detail::is_stl_range<R>>>
constexpr auto to_flow(R&& range)
{
    using rng_t =
        std::conditional_t<std::is_lvalue_reference_v<R>,
                           detail::range_ref<std::remove_reference_t<R>>, R>;

    if constexpr (detail::is_random_access_stl_range<R>) {
        return detail::stl_ra_range_adaptor<rng_t>{FLOW_FWD(range)};
    } else if constexpr (detail::is_bidir_stl_range<R>) {
        return detail::stl_bidir_range_adaptor<rng_t>{FLOW_FWD(range)};
    } else if constexpr (detail::is_fwd_stl_range<R>) {
        return detail::stl_fwd_range_adaptor<rng_t>{FLOW_FWD(range)};
    } else {
        static_assert(
            !std::is_lvalue_reference_v<R>,
            "Input ranges must be passed as rvalues -- use std::move()");
        return detail::stl_input_range_adaptor<R>{FLOW_FWD(range)};
    }
}

template <typename, typename = void>
inline constexpr bool has_member_to_flow = false;

template <typename T>
inline constexpr bool has_member_to_flow<
    T, std::enable_if_t<is_flow<decltype(std::declval<T>().to_flow())>>> = true;

template <typename, typename = void>
inline constexpr bool has_adl_to_flow = false;

template <typename T>
inline constexpr bool has_adl_to_flow<
    T, std::enable_if_t<is_flow<decltype(to_flow(std::declval<T>()))>>> = true;

} // namespace detail

template <typename T>
inline constexpr bool is_flowable =
    detail::has_member_to_flow<T> || detail::has_adl_to_flow<T>;

namespace detail {

struct from_fn {
    template <typename T, typename = std::enable_if_t<is_flowable<T>>>
    constexpr auto operator()(T&& t) const  -> decltype(auto)
    {
        if constexpr (detail::has_member_to_flow<T>) {
            return FLOW_FWD(t).to_flow();
        } else if constexpr (detail::has_adl_to_flow<T>) {
            return to_flow(FLOW_FWD(t));
        }
    }
};

} // namespace detail

inline constexpr detail::from_fn from{};

template <typename T>
using flow_t = decltype(from(std::declval<T>()));

template <typename T>
using flow_item_t = item_t<flow_t<T>>;

template <typename T>
using flow_value_t = value_t<flow_t<T>>;

} // namespace flow

#endif
