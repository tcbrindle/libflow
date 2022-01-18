
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_CORE_MAYBE_HPP_INCLUDED
#define FLOW_CORE_MAYBE_HPP_INCLUDED

#include <flow/core/functional.hpp>

#include <exception>

namespace flow {

struct bad_maybe_access : std::exception {
    bad_maybe_access() = default;

    [[nodiscard]] const char* what() const noexcept override
    {
        return "value() called on an empty maybe";
    }
};

namespace detail {

struct in_place_t {
    constexpr explicit in_place_t() = default;
};

// Storage for the non-trivial case
template <typename T, bool = std::is_trivially_destructible_v<T>>
struct maybe_storage_base {

    constexpr maybe_storage_base() noexcept
        : dummy_{},
          has_value_(false)
    {}

    template <typename... Args>
    constexpr maybe_storage_base(in_place_t, Args&&... args)
        noexcept(std::is_nothrow_constructible_v<T, Args&&...>)
        : value_(FLOW_FWD(args)...),
          has_value_(true)
    {}

    ~maybe_storage_base()
    {
        if (has_value_) {
            value_.~T();
        }
    }

    struct dummy {};

    union {
        dummy dummy_{};
        T value_;
    };

    bool has_value_ = false;
};

// Storage for the trivially-destructible case
template <typename T>
struct maybe_storage_base<T, /*is_trivially_destructible =*/ true>
{
    constexpr maybe_storage_base() noexcept
        : dummy_{},
          has_value_(false)
    {}

    template <typename... Args>
    constexpr maybe_storage_base(in_place_t, Args&&... args)
        noexcept(std::is_nothrow_constructible_v<T, Args&&...>)
        : value_(FLOW_FWD(args)...),
          has_value_(true)
    {}

    struct dummy {};

    union {
        dummy dummy_{};
        T value_;
    };

    bool has_value_ = false;
};

template <typename T>
struct maybe_ops_base : maybe_storage_base<T>
{
    using maybe_storage_base<T>::maybe_storage_base;

    constexpr void hard_reset()
    {
        get().~T();
        this->has_value_ = false;
    }

    template <typename... Args>
    void construct(Args&&... args)
    {
        new (std::addressof(this->value_)) T(FLOW_FWD(args)...);
        this->has_value_ = true;
    }

    template <typename Maybe>
    constexpr void assign(Maybe&& other)
    {
        if (this->has_value()) {
            if (other.has_value()) {
                this->value_ = FLOW_FWD(other).get();
            } else {
                hard_reset();
            }
        } else if (other.has_value()){
            construct(FLOW_FWD(other).get());
        }
    }

    constexpr bool has_value() const
    {
        return this->has_value_;
    }

    constexpr T& get() & { return this->value_; }
    constexpr T const& get() const& { return this->value_; }
    constexpr T&& get() && { return std::move(this->value_); }
    constexpr T&& get() const&& { return std::move(this->value); }
};

template <typename T, bool = std::is_trivially_copy_constructible_v<T>>
struct maybe_copy_base : maybe_ops_base<T>
{
    using maybe_ops_base<T>::maybe_ops_base;

    maybe_copy_base() = default;

    constexpr maybe_copy_base(maybe_copy_base const& other)
    {
        if (other.has_value()) {
            this->construct(other.get());
        } else {
            this->has_value_ = false;
        }
    }

    maybe_copy_base(maybe_copy_base&&) = default;
    maybe_copy_base& operator=(maybe_copy_base const&) = default;
    maybe_copy_base& operator=(maybe_copy_base&&) = default;
};

template <typename T>
struct maybe_copy_base<T, /*std::is_trivially_copy_constructible =*/ true>
    : maybe_ops_base<T>
{
    using maybe_ops_base<T>::maybe_ops_base;
};


template <typename T, bool = std::is_trivially_move_constructible_v<T>>
struct maybe_move_base : maybe_copy_base<T>
{
    using maybe_copy_base<T>::maybe_copy_base;

    maybe_move_base() = default;

    maybe_move_base(maybe_move_base const&) = default;

    constexpr maybe_move_base(maybe_move_base&& other)
    noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        if (other.has_value()) {
            this->construct(std::move(other).get());
        } else {
            this->has_value_ = false;
        }
    }

    maybe_move_base& operator=(maybe_move_base const&) = default;
    maybe_move_base& operator=(maybe_move_base&&) = default;
};

template <typename T>
struct maybe_move_base<T, /*is_trivially_move_constructible = */true>
    : maybe_copy_base<T>
{
    using maybe_copy_base<T>::maybe_copy_base;
};

template <typename T, bool = std::is_trivially_copy_assignable_v<T>>
struct maybe_copy_assign_base : maybe_move_base<T>
{
    using maybe_move_base<T>::maybe_move_base;

    maybe_copy_assign_base() = default;

    maybe_copy_assign_base(maybe_copy_assign_base const&) = default;

    maybe_copy_assign_base(maybe_copy_assign_base&&) = default;

    constexpr maybe_copy_assign_base&
    operator=(maybe_copy_assign_base const& other)
    {
        this->assign(other);
        return *this;
    }

    maybe_copy_assign_base& operator=(maybe_copy_assign_base&&) = default;
};

template <typename T>
struct maybe_copy_assign_base<T, /*is_trivially_copy_assignable = */true>
    : maybe_move_base<T>
{
    using maybe_move_base<T>::maybe_move_base;
};

template <typename T, bool = std::is_trivially_move_assignable_v<T>>
struct maybe_move_assign_base : maybe_copy_assign_base<T>
{
    using maybe_copy_assign_base<T>::maybe_copy_assign_base;

    maybe_move_assign_base() = default;

    maybe_move_assign_base(maybe_move_assign_base const&) = default;

    maybe_move_assign_base(maybe_move_assign_base&&) = default;

    maybe_move_assign_base& operator=(maybe_move_assign_base const&) = default;

    constexpr maybe_move_assign_base& operator=(maybe_move_assign_base&& other)
    noexcept(std::is_nothrow_move_assignable_v<T> &&
        std::is_nothrow_move_assignable_v<T>)
    {
        this->assign(std::move(other));
        return *this;
    }
};

template <typename T>
struct maybe_move_assign_base<T, /*is_trivially_move_assignable = */ true>
    : maybe_copy_assign_base<T>
{
    using maybe_copy_assign_base<T>::maybe_copy_assign_base;
};

template <typename T,
    bool EnableCopy = std::is_copy_constructible_v<T>,
    bool EnableMove = std::is_move_constructible_v<T>>
struct maybe_delete_ctor_base {
    maybe_delete_ctor_base() = default;
    maybe_delete_ctor_base(maybe_delete_ctor_base const&) = default;
    maybe_delete_ctor_base(maybe_delete_ctor_base&&) = default;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base const&) = default;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base&&) = default;
};

template <typename T>
struct maybe_delete_ctor_base<T, false, true> {
    maybe_delete_ctor_base() = default;
    maybe_delete_ctor_base(maybe_delete_ctor_base const&) = delete;
    maybe_delete_ctor_base(maybe_delete_ctor_base&&) = default;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base const&) = default;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base&&) = default;
};

template <typename T>
struct maybe_delete_ctor_base<T, true, false> {
    maybe_delete_ctor_base() = default;
    maybe_delete_ctor_base(maybe_delete_ctor_base const&) = default;
    maybe_delete_ctor_base(maybe_delete_ctor_base&&) = delete;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base const&) = default;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base&&) = default;
};

template <typename T>
struct maybe_delete_ctor_base<T, false, false> {
    maybe_delete_ctor_base() = default;
    maybe_delete_ctor_base(maybe_delete_ctor_base const&) = delete;
    maybe_delete_ctor_base(maybe_delete_ctor_base&&) = delete;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base const&) = default;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base&&) = default;
};

template <typename T,
    bool EnableCopy = std::is_copy_assignable_v<T>,
    bool EnableMove = std::is_move_assignable_v<T>>
struct maybe_delete_assign_base {
    maybe_delete_assign_base() = default;
    maybe_delete_assign_base(maybe_delete_assign_base const&) = default;
    maybe_delete_assign_base(maybe_delete_assign_base&&) = default;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base const&) = default;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base&&) = default;
};

template <typename T>
struct maybe_delete_assign_base<T, false, true> {
    maybe_delete_assign_base() = default;
    maybe_delete_assign_base(maybe_delete_assign_base const&) = default;
    maybe_delete_assign_base(maybe_delete_assign_base&&) = default;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base const&) = delete;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base&&) = default;
};

template <typename T>
struct maybe_delete_assign_base<T, true, false> {
    maybe_delete_assign_base() = default;
    maybe_delete_assign_base(maybe_delete_assign_base const&) = default;
    maybe_delete_assign_base(maybe_delete_assign_base&&) = default;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base const&) = default;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base&&) = delete;
};

template <typename T>
struct maybe_delete_assign_base<T, false, false> {
    maybe_delete_assign_base() = default;
    maybe_delete_assign_base(maybe_delete_assign_base const&) = default;
    maybe_delete_assign_base(maybe_delete_assign_base&&) = default;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base const&) = delete;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base&&) = delete;
};

} // namespace detail

template <typename T>
class maybe : detail::maybe_move_assign_base<T>,
              detail::maybe_delete_ctor_base<T>,
              detail::maybe_delete_assign_base<T>
{
    using base = detail::maybe_move_assign_base<T>;

public:
    using value_type = T;

    maybe() = default;

    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    constexpr maybe(U const& other)
        : base(detail::in_place_t{}, other)
    {}

    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    constexpr maybe(U&& other)
        : base(detail::in_place_t{}, FLOW_FWD(other))
    {}

    constexpr T& operator*() & { return this->get(); }
    constexpr T const& operator*() const& { return this->get(); }
    constexpr T&& operator*() && { return std::move(this->get()); }
    constexpr T const&& operator*() const&& { return std::move(this->get()); }

    constexpr T* operator->() { return std::addressof(**this); }
    constexpr T const* operator->() const { return std::addressof(**this); }

    constexpr T& value() & { return value_impl(*this); }
    constexpr T const& value() const& { return value_impl(*this); }
    constexpr T&& value() && { return value_impl(std::move(*this)); }
    constexpr T const&& value() const&& { return value_impl(std::move(*this)); }

    using base::has_value;

    constexpr explicit operator bool() const { return has_value(); }

    constexpr void reset() { this->hard_reset(); }

    // I really want "deducing this"...
    template <typename U>
    constexpr auto value_or(U&& arg) &
        -> decltype(has_value() ? std::declval<T&>() : FLOW_FWD(arg))
    {
        return has_value() ? **this : FLOW_FWD(arg);
    }

    template <typename U>
    constexpr auto value_or(U&& arg) const&
        -> decltype(has_value() ? std::declval<T const&>() : FLOW_FWD(arg))
    {
        return has_value() ? **this : FLOW_FWD(arg);
    }

    template <typename U>
    constexpr auto value_or(U&& arg) &&
        -> decltype(has_value() ? std::declval<T&&>() : FLOW_FWD(arg))
    {
        return has_value() ? *std::move(*this) : FLOW_FWD(arg);
    }

    template <typename U>
    constexpr auto value_or(U&& arg) const&&
        -> decltype(has_value() ? std::declval<T const&&>() : FLOW_FWD(arg))
    {
        return has_value() ? *std::move(*this) : FLOW_FWD(arg);
    }

    template <typename Func>
    constexpr auto map(Func&& func) & { return map_impl(*this, FLOW_FWD(func)); }
    template <typename Func>
    constexpr auto map(Func&& func) const& { return map_impl(*this, FLOW_FWD(func)); }
    template <typename Func>
    constexpr auto map(Func&& func) && { return map_impl(std::move(*this),  FLOW_FWD(func)); }
    template <typename Func>
    constexpr auto map(Func&& func) const&& { return map_impl(std::move(*this), FLOW_FWD(func)); }

private:
    template <typename Self>
    constexpr static auto value_impl(Self&& self) -> decltype(auto)
    {
        if (self.has_value()) {
            return *FLOW_FWD(self);
        } else {
            throw bad_maybe_access{};
        }
    }

    template <typename Self, typename Func>
    constexpr static auto map_impl(Self&& self, Func&& func)
        -> maybe<std::invoke_result_t<Func, decltype(*FLOW_FWD(self))>>
    {
        if (self.has_value()) {
            return {FLOW_FWD(func)(*FLOW_FWD(self))};
        } else {
            return {};
        }
    }
};


template <typename T>
class maybe<T&> {
    T* ptr_ = nullptr;

public:
    using value_type = T&;

    maybe() = default;

    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    constexpr maybe(U& item) : ptr_(std::addressof(item)) {}

    maybe(T&&) = delete;

    constexpr T& operator*() const { return *ptr_; }
    constexpr T* operator->() const { return ptr_; }

    constexpr T& value() const
    {
        if (!ptr_) {
            throw bad_maybe_access{};
        }
        return *ptr_;
    }

    template <typename U>
    constexpr auto value_or(U&& arg) const
    -> decltype(ptr_ ? *ptr_ : FLOW_FWD(arg))
    {
        return ptr_ ? *ptr_ : FLOW_FWD(arg);
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
};

} // namespace flow

#endif
