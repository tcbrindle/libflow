
#ifndef FLOW_OP_TO_HPP_INCLUDED
#define FLOW_OP_TO_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Void, template <typename...> typename C, typename... Args>
inline constexpr bool is_ctad_constructible_v = false;

template <template <typename...> typename C, typename... Args>
inline constexpr bool is_ctad_constructible_v<
    std::void_t<decltype(C(std::declval<Args>()...))>, C, Args...> = true;

}

// These need to be function templates so that the user can supply the template
// arg, and we can overload on both type template parameters and
// template template params
template <typename C, typename Flow>
constexpr auto to(Flow&& flow) -> C
{
    return FLOW_FWD(flow).template to<C>();
}

template <template <typename...> typename C, typename Flow>
constexpr auto to(Flow&& flow)
{
    return FLOW_FWD(flow).template to<C>();
}

template <typename Flow>
auto to_vector(Flow&& flow) -> std::vector<value_t<Flow>>
{
    return FLOW_FWD(flow).to_vector();
}

template <typename T, typename Flow>
auto to_vector(Flow&& flow) -> std::vector<T>
{
    return FLOW_FWD(flow).template to_vector<T>();
}

template <typename Flow>
auto to_string(Flow&& flow) -> std::string
{
    return FLOW_FWD(flow).to_string();
}

template <typename D>
template <typename C>
constexpr auto flow_base<D>::to() && -> C
{
    if constexpr (std::is_constructible_v<C, D&&>) {
        return C(consume());
    } else {
        auto rng = consume().to_range();
        static_assert(std::is_constructible_v<C, decltype(rng.begin()),
                                              decltype(rng.end())>);
        return C(rng.begin(), rng.end());
    }
}

template <typename D>
template <template <typename...> typename C>
constexpr auto flow_base<D>::to() &&
{
    if constexpr (detail::is_ctad_constructible_v<void, C, D&&>) {
        return C(consume());
    } else {
        auto rng = consume().to_range();
        static_assert(detail::is_ctad_constructible_v<void, C,
                      decltype(rng.begin()), decltype(rng.end())>);
        return C(rng.begin(), rng.end());
    }
}

template <typename D>
template <typename T>
auto flow_base<D>::to_vector() && -> std::vector<T>
{
    return consume().template to<std::vector<T>>();
}

template <typename D>
auto flow_base<D>::to_vector() &&
{
    return consume().template to<std::vector<value_t<D>>>();
}

template <typename D>
auto flow_base<D>::to_string() && -> std::string
{
    return consume().template to<std::string>();
}

}

#endif

