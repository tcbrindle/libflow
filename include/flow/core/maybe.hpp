
#ifndef FLOW_CORE_MAYBE_HPP_INCLUDED
#define FLOW_CORE_MAYBE_HPP_INCLUDED

#include <flow/core/functional.hpp>

#include <optional>

namespace flow {

template <typename>
class optional;

template <typename>
struct optional_ref;

template <typename T>
using maybe = std::conditional_t<
    std::is_lvalue_reference_v<T>,
        optional_ref<std::remove_reference_t<T>>,
        optional<std::remove_const_t<std::remove_reference_t<T>>>>;

template <typename T>
class optional : std::optional<T>  {

    using base = std::optional<T>;

public:
    using value_type = T;

    optional() = default;

    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    constexpr optional(const U& value) : base{std::in_place, value} {}

    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    constexpr optional(U&& value) : base{std::in_place, FLOW_FWD(value)} {}

    using base::operator*;
    using base::operator->;
    using base::value;
    using base::has_value;
    using base::reset;

#ifndef _MSC_VER
    using std::optional<T>::operator bool;
#else
    constexpr explicit operator bool() const {
        return this->has_value();
    }
#endif

    template <typename Func>
    constexpr auto map(Func&& func) & { return map_impl(*this, FLOW_FWD(func)); }
    template <typename Func>
    constexpr auto map(Func&& func) const& { return map_impl(*this, FLOW_FWD(func)); }
    template <typename Func>
    constexpr auto map(Func&& func) && { return map_impl(std::move(*this), FLOW_FWD(func)); }
    template <typename Func>
    constexpr auto map(Func&& func) const&& { return map_impl(std::move(*this), FLOW_FWD(func)); }

private:
    template <typename Self, typename F>
    static constexpr auto map_impl(Self&& self, F&& func)
        -> maybe<std::invoke_result_t<F, decltype(*FLOW_FWD(self))>>
    {
        if (self) {
            return invoke(FLOW_FWD(func), *FLOW_FWD(self));
        }
        return {};
    }
};

template <typename T>
struct optional_ref {
    using value_type = T&;

    optional_ref() = default;

    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    constexpr optional_ref(U& item) : ptr_(std::addressof(item)) {}

    optional_ref(T&&) = delete;

    constexpr T& operator*() const { return *ptr_; }
    constexpr T* operator->() const { return ptr_; }

    constexpr T& value() const
    {
        if (!ptr_) {
            throw std::bad_optional_access{};
        }
        return *ptr_;
    }

    constexpr auto reset() { ptr_ = nullptr; }

    constexpr auto has_value() const -> bool { return ptr_ != nullptr; }
    explicit constexpr operator bool() const { return has_value(); }

    template <typename Func>
    constexpr auto map(Func&& func) const -> maybe<std::invoke_result_t<Func, T&>>
    {
        if (ptr_) {
            return {invoke(FLOW_FWD(func), *ptr_)};
        }
        return {};
    }

private:
    T* ptr_ = nullptr;
};

} // namespace flow

#endif
