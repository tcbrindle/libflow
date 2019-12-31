
#ifndef FLOW_CORE_MAYBE_HPP_INCLUDED
#define FLOW_CORE_MAYBE_HPP_INCLUDED

#include <optional>

namespace flow {

namespace detail {

template <typename T>
struct maybe_move_assign_base : std::optional<T>
{
    using std::optional<T>::optional;

    constexpr maybe_move_assign_base() = default;
    constexpr maybe_move_assign_base(maybe_move_assign_base&&) = default;
    constexpr maybe_move_assign_base(maybe_move_assign_base const&) = default;

    ~maybe_move_assign_base() = default;

    constexpr maybe_move_assign_base& operator=(maybe_move_assign_base&& other)
        noexcept(noexcept(std::is_nothrow_move_constructible_v<T>))
    {
        if (other) {
            as_opt() = std::optional<T>{std::in_place, *std::move(other)};
        } else {
            as_opt() = std::optional<T>{std::nullopt};
        }
        return *this;
    }

protected:
    constexpr auto& as_opt() & { return static_cast<std::optional<T>&>(*this); }
    constexpr auto& as_opt() const& { return static_cast<std::optional<T> const&>(*this); }
};

template <typename T, bool IsCopyable = std::is_copy_constructible_v<T>>
struct maybe_copy_assign_base : maybe_move_assign_base<T> {

    using maybe_move_assign_base<T>::maybe_move_assign_base;

    constexpr maybe_copy_assign_base() = default;
    constexpr maybe_copy_assign_base(maybe_copy_assign_base&&) = default;
//    constexpr maybe_copy_assign_base& operator=(maybe_copy_assign_base&&) = default;
    // Urgh, Clang doesn't think the defaulted definition is constexpr for some reason
    constexpr maybe_copy_assign_base& operator=(maybe_copy_assign_base&& other)
        noexcept(std::is_nothrow_move_assignable_v<maybe_move_assign_base<T>>)
    {
        static_cast<maybe_move_assign_base<T>&>(*this)
                = static_cast<maybe_move_assign_base<T>&&>(other);
        return *this;
    }

    constexpr maybe_copy_assign_base(maybe_copy_assign_base const&) = default;
    ~maybe_copy_assign_base() = default;

    constexpr maybe_copy_assign_base& operator=(maybe_copy_assign_base const& other)
    {
        if (this != &other) {
            if (other.as_opt()) {
                this->as_opt() = std::optional<T>{std::in_place, *other.as_opt()};
            } else {
                this->as_opt() = std::optional<T>{std::nullopt};
            }
        }
        return *this;
    }
};

template <typename T>
struct maybe_copy_assign_base<T, false> : maybe_move_assign_base<T> {
    using maybe_move_assign_base<T>::maybe_move_assign_base;
};

}

template <typename T>
class maybe : detail::maybe_copy_assign_base<T>  {

    using base = detail::maybe_copy_assign_base<T>;

public:
    using value_type = T;

    constexpr maybe() = default;

    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    constexpr maybe(const U& value) : base{std::in_place, value} {}

    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    constexpr maybe(U&& value) : base{std::in_place, std::move(value)} {}

    constexpr maybe(maybe const&) = default;
    constexpr maybe& operator=(const maybe&) = default;
    constexpr maybe(maybe&&) = default;
    constexpr maybe& operator=(maybe&&) = default;

    using std::optional<T>::operator*;
    using std::optional<T>::operator->;
    using std::optional<T>::value;
    using std::optional<T>::reset;
#ifndef _MSC_VER
    using std::optional<T>::operator bool;
#else
    constexpr explicit operator bool() const {
        return this->has_value();
    }
#endif

    template <typename Func>
    constexpr auto map(Func&& func) & { return maybe::map_impl(*this, FLOW_FWD(func)); }
    template <typename Func>
    constexpr auto map(Func&& func) const& { return maybe::map_impl(*this, FLOW_FWD(func)); }
    template <typename Func>
    constexpr auto map(Func&& func) && { return maybe::map_impl(std::move(*this), FLOW_FWD(func)); }
    template <typename Func>
    constexpr auto map(Func&& func) const&& { return maybe::map_impl(std::move(*this), FLOW_FWD(func)); }

private:
    template <typename Self, typename F>
    static constexpr auto map_impl(Self&& self, F&& func)
        -> maybe<decltype(FLOW_FWD(func)(*FLOW_FWD(self)))>
    {
        if (self) {
            return FLOW_FWD(func)(*FLOW_FWD(self));
        }
        return {};
    }
};

template <typename T>
struct maybe<T&> {
    using value_type = T&;

    constexpr maybe() = default;

    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    constexpr maybe(U& item) : ptr_(std::addressof(item)) {}

    maybe(T&&) = delete;

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

    explicit constexpr operator bool() const { return ptr_ != nullptr; }
#if 0
    template <typename Func>
    constexpr auto map(Func&& func) -> maybe<std::invoke_result_t<Func, T&>>
    {
        if (ptr_) {
            return {FLOW_FWD(func)(*ptr_)};
        }
        return {};
    }
#endif

    template <typename Func>
    constexpr auto map(Func&& func) const -> maybe<std::invoke_result_t<Func, T&>>
    {
        if (ptr_) {
            return {FLOW_FWD(func)(*ptr_)};
        }
        return {};
    }

private:
    T* ptr_ = nullptr;
};

template <typename T>
struct maybe<T&&>
{
    using value_type = T&&;

    constexpr maybe() = default;

    template <typename U, std::enable_if_t<
        std::is_same_v<std::remove_reference_t<T>, std::remove_reference_t<U>>, int> = 0>
    constexpr maybe(U&& item) : ptr_(std::addressof(item)) {}

//    maybe(T&&) = delete;

    constexpr T& operator*() const & { return *ptr_; }
    constexpr T&& operator*() const && { return std::move(*ptr_); }

    constexpr auto reset() { ptr_ = nullptr; }

    explicit constexpr operator bool() const { return ptr_ != nullptr; }

    template <typename Func>
    constexpr auto map(Func&& func) const & -> maybe<std::invoke_result_t<Func, T&>>
    {
        if (ptr_) {
            return {FLOW_FWD(func)(**this)};
        }
        return {};
    }

    template <typename Func>
    constexpr auto map(Func&& func) const && -> maybe<std::invoke_result_t<Func, T&&>>
    {
        if (ptr_) {
            return {FLOW_FWD(func)(*std::move(*this))};
        }
        return {};
    }

private:
    T* ptr_;
};

} // namespace flow

#endif