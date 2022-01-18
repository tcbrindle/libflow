
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_SOURCE_IOTA_HPP_INCLUDED
#define FLOW_SOURCE_IOTA_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

inline constexpr auto pre_inc = [] (auto& val)
    -> decltype(val++) { return val++; };

inline constexpr auto iota_size_fn = [](auto const& val, auto const& bound)
    -> decltype(bound - val)
{
    return bound - val;
};

inline constexpr auto stepped_iota_size_fn =
    [](auto const& val, auto const& bound, auto const& step)
    -> decltype((bound - val)/step + ((bound - val) % step != 0))
{
    return (bound - val)/step + ((bound - val) % step != 0);
};

template <typename Val>
struct iota_flow : flow_base<iota_flow<Val>> {
    static constexpr bool is_infinite = true;

    constexpr iota_flow(Val&& val)
        : val_(std::move(val))
    {}

    constexpr auto next() -> maybe<Val>
    {
        return {val_++};
    }

private:
    Val val_;
};

template <typename Val, typename Bound>
struct bounded_iota_flow : flow_base<bounded_iota_flow<Val, Bound>> {

    constexpr bounded_iota_flow(Val&& val, Bound&& bound)
        : val_(std::move(val)),
          bound_(std::move(bound))
    {}

    constexpr auto next() -> maybe<Val>
    {
        if (val_ < bound_) {
            return {val_++};
        }
        return {};
    }

    template <bool B = std::is_same_v<Val, Bound>>
    constexpr auto next_back() -> std::enable_if_t<B, maybe<Val>>
    {
        if (val_ < bound_) {
            return {--bound_};
        }
        return {};
    }

    template <typename V = const Val&, typename B = const Bound&,
              typename = std::enable_if_t<
                  std::is_invocable_r_v<flow::dist_t, decltype(iota_size_fn), V, B>>>
    [[nodiscard]] constexpr auto size() const -> dist_t
    {
        return iota_size_fn(val_, bound_);
    }

private:
    Val val_;
    FLOW_NO_UNIQUE_ADDRESS Bound bound_;
};

template <typename Val, typename Bound, typename Step>
struct stepped_iota_flow : flow_base<stepped_iota_flow<Val, Bound, Step>> {
private:
    Val val_;
    Bound bound_;
    Step step_;
    bool step_positive_ = step_ > Step{};

public:
    constexpr stepped_iota_flow(Val val, Bound bound, Step step)
        : val_(std::move(val)),
          bound_(std::move(bound)),
          step_(std::move(step))
    {}

    constexpr auto next() -> maybe<Val>
    {
        if (step_positive_) {
            if (val_ >= bound_) {
                return {};
            }
        } else {
            if (val_ <= bound_) {
                return {};
            }
        }

        auto tmp = val_;
        val_ += step_;
        return {std::move(tmp)};
    }

    template <typename V = Val, typename B = Bound, typename S = Step,
              typename = std::enable_if_t<
                  std::is_invocable_r_v<dist_t, decltype(stepped_iota_size_fn), V&, B&, S&>
                  >>
    [[nodiscard]] constexpr auto size() const -> dist_t
    {
        return static_cast<dist_t>(stepped_iota_size_fn(val_, bound_, step_));
    }


};

struct iota_fn {

    template <typename Val>
    constexpr auto operator()(Val from) const
    {
        static_assert(std::is_invocable_v<decltype(pre_inc), Val&>,
                      "Type passed to flow::iota() must have a pre-increment operator++(int)");
        return iota_flow<Val>(std::move(from));
    }

    template <typename Val, typename Bound>
    constexpr auto operator()(Val from, Bound upto) const
    {
        static_assert(std::is_invocable_v<decltype(pre_inc), Val&>,
                      "Type passed to flow::iota() must have a pre-increment operator++(int)");
        return bounded_iota_flow<Val, Bound>(std::move(from), std::move(upto));
    }

    template <typename Val, typename Bound, typename Step>
    constexpr auto operator()(Val from, Bound upto, Step step) const
    {
        return stepped_iota_flow<Val, Bound, Step>(
            std::move(from), std::move(upto), std::move(step));
    }
};

struct ints_fn {

    constexpr auto operator()() const
    {
        return iota_fn{}(dist_t{0});
    }

    constexpr auto operator()(dist_t from) const
    {
        return iota_fn{}(from);
    }

    constexpr auto operator()(dist_t from, dist_t upto) const
    {
        return iota_fn{}(from, upto);
    }

    constexpr auto operator()(dist_t from, dist_t upto, dist_t step) const
    {
        return iota_fn{}(from, upto, step);
    }

};

} // namespace detail

constexpr auto iota = detail::iota_fn{};

constexpr auto ints = detail::ints_fn{};

}

#endif
