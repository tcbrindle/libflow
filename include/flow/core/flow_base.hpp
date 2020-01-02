
#ifndef FLOW_CORE_FLOW_BASE_HPP_INCLUDED
#define FLOW_CORE_FLOW_BASE_HPP_INCLUDED

#include <flow/core/invoke.hpp>
#include <flow/core/maybe.hpp>
#include <flow/core/type_traits.hpp>

#include <iosfwd>  // for stream_to()
#include <string>  // for to_string()
#include <vector>  // for to_vector()

namespace flow {

template <typename Derived>
struct flow_base {
private:
    constexpr auto derived() & -> Derived& { return static_cast<Derived&>(*this); }
    constexpr auto derived() && -> Derived&& { return static_cast<Derived&&>(*this); }
    constexpr auto consume() -> Derived&& { return std::move(*this).derived(); }

protected:
    ~flow_base() = default;

public:
    template <typename D = Derived>
    constexpr auto advance(dist_t dist) -> maybe<item_t<D>>
    {
        for (dist_t i = 0; i < dist - 1; i++) {
            derived().next();
        }

        return derived().next();
    }

    template <typename Func, typename Init>
    constexpr auto try_fold(Func func, Init init) -> Init
    {
        while (auto m = derived().next()) {
            init = invoke(func, std::move(init), std::move(m));
            if (!init) {
                break;
            }
        }
        return init;
    }

    template <typename Func>
    constexpr auto try_for_each(Func func)
    {
        using result_t = std::invoke_result_t<Func&, maybe<item_t<Derived>>&&>;
        return consume().try_fold([&func](auto&& e, auto m) {
            return invoke(func, std::move(m));
        }, result_t{});
    }

    template <typename Adaptor, typename... Args>
    constexpr auto apply(Adaptor&& adaptor, Args&&... args) && -> decltype(auto)
    {
        return invoke(FLOW_FWD(adaptor), consume(), FLOW_FWD(args)...);
    }

    // Reductions of various kinds
    template <typename Func, typename Init>
    constexpr auto fold(Func func, Init init) && -> Init;

    template <typename Func>
    constexpr auto fold(Func func) &&;

    template <typename Func>
    constexpr auto for_each(Func func) &&;

    // Consumes the flow, returning the number of items for which @pred
    // returned true
    template <typename Pred>
    constexpr auto count_if(Pred pred) && -> dist_t;

    /// Consumes the flow, returning the number of items it contained
    constexpr auto count() && -> dist_t
    {
        return consume().count_if([](auto&& /*unused*/) { return true; });
    }

    /// Consumes the flow, returning a count of the number of items which
    /// compared equal to @item, using comparator @cmp
    template <typename T, typename Cmp = std::equal_to<>>
    constexpr auto count(const T& item, Cmp cmp = {}) && -> dist_t
    {
        return consume().count_if([&item, &cmp] (auto&& val) {
            return invoke(cmp, FLOW_FWD(val), item);
        });
    }

    template <typename T, typename Cmp = std::equal_to<>>
    constexpr auto find(const T& item, Cmp cmp = {})
    {
        struct out {
           maybe<item_t<Derived>> val{};
           constexpr explicit operator bool() const { return !val; }
       };

       return consume().try_for_each([&item, &cmp](auto m) {
             return invoke(cmp, *m, item) ? out{std::move(m)} : out{};
       }).val;
    }

    /// Returns true if the flow contains the given item, or false otherwise
    /// This function is short-circuiting: it will stop looking for an item
    /// as soon as it finds one
    template <typename T, typename Cmp = std::equal_to<>>
    constexpr auto contains(const T& item, Cmp cmp = {}) -> bool
    {
        return static_cast<bool>(consume().find(item, std::move(cmp)));
    }

    /// Consumes the flow, returning the sum of elements using `operator+`
    template <typename D = Derived>
    constexpr auto sum() && -> value_t<D>
    {
        return consume().fold(std::plus<>{});
    }

    /// Consumes the flow, returning the product of the elements using `operator*`
    template <typename D = Derived>
    constexpr auto product() && -> value_t<D>
    {
        static_assert(std::is_constructible_v<value_t<Derived>, int>,
            "Flow's value type must be constructible from a literal 1");
        return consume().fold(std::multiplies<>{}, value_t<Derived>{1});
    }

    /// Consumes the flow, returning the smallest element.
    /// If the flow is empty, return an empty `maybe`
    template <typename Cmp = std::less<>>
    constexpr auto min(Cmp cmp = Cmp{}) &&
    {
        return consume().next().map([this, &cmp] (auto&& init) {
          return consume().fold([&cmp](auto&& val, auto&& acc) -> decltype(auto) {
            return invoke(cmp, val, acc) ? FLOW_FWD(val) : FLOW_FWD(acc);
          }, FLOW_FWD(init));
        });
    }

    /// Consumes the flow, returning the largest element.
    /// If the flow is empty, return an empty `maybe`
    template <typename Cmp = std::less<>>
    constexpr auto max(Cmp cmp = Cmp{}) &&
    {
        return consume().next().map([this, &cmp] (auto&& init) {
            return consume().fold([&cmp](auto&& val, auto acc) {
                return !invoke(cmp, val, acc) ? FLOW_FWD(val) : std::move(acc);
            }, FLOW_FWD(init));
        });
    }

    /// Iterates over the flow, returning true if all the items satisfy the predicate
    /// Returns true for an empty flow
    template <typename Pred>
    constexpr auto all(Pred pred) -> bool
    {
        static_assert(std::is_invocable_r_v<bool, Pred&, item_t<Derived>>,
                      "Predicate must be callable with the Flow's item_type,"
                      " and must return bool");
        return consume().try_fold([&pred](bool acc, auto m) {
            return acc && invoke(pred, *std::move(m));
        }, true);
    }

    /// Iterates over the flow, returning false if any element satisfies the predicate.
    /// Returns true for an empty flow
    template <typename Pred>
    constexpr auto none(Pred pred) -> bool
    {
        static_assert(std::is_invocable_r_v<bool, Pred&, item_t<Derived>>,
                      "Predicate must be callable with the Flow's item_type,"
                      " and must return bool");
        return consume().all([&pred] (auto&& val) {
            return !invoke(pred, FLOW_FWD(val));
        });
    }

    /// Iterates over the flow, returning true if any element satisfies the predicate.
    /// Returns false for an empty flow
    template <typename Pred>
    constexpr auto any(Pred pred) -> bool
    {
        return !consume().none(std::move(pred));
    }

    /// Consumes the flow, returning true if the elements are sorted according to @cmp.
    template <typename Cmp = std::less<>>
    constexpr auto is_sorted(Cmp cmp = Cmp{}) && -> bool
    {
        return consume().try_fold([last = derived().next(), &cmp]
        (auto acc, auto next) mutable {
            if (invoke(cmp, *next, *last)) {
                return false;
            } else {
                last = std::move(next);
                return true;
            }
        }, true);

    }

    template <typename NextFn>
    constexpr auto adapt(NextFn next_fn) &&;

    template <typename Func>
    constexpr auto map(Func func) &&
    {
        static_assert(std::is_invocable_v<Func&, item_t<Derived>&&>,
            "Incompatible callable passed to map()");
        static_assert(!std::is_void_v<std::invoke_result_t<Func&, item_t<Derived>&&>>,
            "May cannot be used with a function returning void");
        return consume().adapt([self = consume(), func = std::move(func)] () mutable {
            return self.next().map(func);
        });
    }

    template <typename = Derived>
    constexpr auto deref() &&
    {
        auto deref = [](auto&& val) -> decltype(auto) { return *val; };
        static_assert(std::is_invocable_v<decltype(deref), item_t<Derived>>,
            "Flow's item type is not dereferenceable with unary operator*");
        static_assert(!std::is_void_v<decltype(*std::declval<item_t<Derived>>())>,
            "Flow's item type dereferences to void");
        return consume().map(deref);
    }

    template <typename T>
    constexpr auto as() &&
    {
        return consume().map([](auto&& val) -> decltype(auto) {
            return static_cast<T>(FLOW_FWD(val));
        });
    }

    template <typename = Derived>
    constexpr auto copy() && -> decltype(auto)
    {
        if constexpr (std::is_lvalue_reference_v<item_t<Derived>>) {
            return consume().template as<value_t<Derived>>();
        } else {
            return consume();
        }
    }

    template <typename = Derived>
    constexpr auto move() && -> decltype(auto)
    {
        if constexpr (std::is_lvalue_reference_v<item_t<Derived>>) {
            return consume()
                .template as<std::remove_reference_t<item_t<Derived>>&&>();
        } else {
            return consume();
        }
    }

    template <typename Pred>
    constexpr auto filter(Pred pred) &&
    {
        static_assert(std::is_invocable_v<Pred&, item_t<Derived>&>,
            "Incompatible predicate passed to filter()");
        constexpr auto is_boolish = [](auto&& p) { return p ? true : false; };
        static_assert(std::is_invocable_v<decltype(is_boolish),
            std::invoke_result_t<Pred&, item_t<Derived>&>>,
            "filter predicate must return a type which is contextutally convertible to bool");

        auto filter_fn = [self = consume(), pred = std::move(pred)] () mutable
            -> maybe<item_t<Derived>>
            {
                while (auto o = self.next()) {
                    if (invoke(pred, *o)) {
                        return o;
                    }
                }
                return {};
            };
        return consume().adapt(std::move(filter_fn));
    }

    template <typename Func>
    constexpr auto filter_map(Func func) &&
    {
        static_assert(std::is_invocable_v<Func&, item_t<Derived>>);
        auto to_bool = [](auto&& a) { return static_cast<bool>(a); };
        using ret_t = std::invoke_result_t<Func&, item_t<Derived>>;
        static_assert(std::is_invocable_v<decltype(to_bool), ret_t&>);

        return consume().map(std::move(func)).filter([] (auto&& val) {
            return val ? true : false;
        }).deref();
    }

    template <typename = Derived>
    constexpr auto drop(dist_t count) &&
    {
        auto drop_fn = [self = consume(), count = count] () mutable {
            if (count > 0) {
                self.advance(count);
                count = 0;
            }
            return self.next();
        };
        return consume().adapt(std::move(drop_fn));
    }

    template <typename Pred>
    constexpr auto drop_while(Pred pred) &&
    {
        auto fn = [self = consume(),
                   pred = std::move(pred),
                   done = false] () mutable -> maybe<item_t<Derived>>
        {
            while (auto m = self.next()) {
                if (!done) {
                    if (invoke(pred, *m)) {
                        continue;
                    } else {
                        done = true;
                    }
                }
                return m;
            }
            return {};
        };
        return consume().adapt(std::move(fn));
    }

    template <typename = Derived>
    constexpr auto take(dist_t count) &&
    {
        auto fn = [self = consume(), count = count] () mutable -> maybe<item_t<Derived>>
        {
            if (count > 0) {
                --count;
                return self.next();
            }
            return {};
        };

        return consume().adapt(std::move(fn));
    }

    template <typename Pred>
    constexpr auto take_while(Pred pred) &&
    {
        auto fn = [self = consume(),
                   pred = std::move(pred),
                   done = false] () mutable -> maybe<item_t<Derived>> {
            if (!done) {
                auto m = self.next();
                if (invoke(pred, *m)) {
                    return m;
                } else {
                    done = true;
                }
            }
            return {};
        };
        return consume().adapt(std::move(fn));
    }

    template <typename = Derived>
    constexpr auto step_by(dist_t stride) &&
    {
        auto fn = [self = consume(), stride = stride] () mutable
        {
            auto m = self.next();
            self.advance(stride - 1);
            return m;
        };
        return consume().adapt(std::move(fn));
    }

    template <typename = Derived>
    constexpr auto cycle() &&
    {
        auto fn = [cur = derived(), initial = consume(), done = false] () mutable
            -> maybe<item_t<Derived>>
        {
            if (!done) {
                auto m = cur.next();
                if (m) {
                    return m;
                } else {
                    cur = initial;
                    auto m2 = cur.next();
                    if (!m2) {
                        done = true;
                    }
                }
            }
            return {};
        };
        return consume().adapt(std::move(fn));
    }

    template <typename Flow>
    constexpr auto chain(Flow other) &&
    {
        static_assert(std::is_same_v<item_t<Derived>, item_t<Flow>>,
            "Flows used with chain() must have the exact same item type");

        auto fn = [self = consume(),
                   other = std::move(other),
                   first = true] () mutable -> maybe<item_t<Derived>> {
          if (first) {
              if (auto m = self.next()) {
                  return m;
              }
              first = false;
          }
          return other.next();
        };
        return consume().adapt(std::move(fn));
    }

    template <typename = Derived>
    constexpr auto flatten() &&;

    template <typename Func>
    constexpr auto flat_map(Func func) &&;

    template <typename Flow>
    constexpr auto zip_with(Flow other) &&
    {
        using item_type = std::pair<item_t<Derived>, item_t<Flow>>;

        auto fn = [self = consume(), other = std::move(other)] () mutable
            -> maybe<item_type>
        {
            auto m1 = self.next();
            if (!m1) { return {}; }

            auto m2 = other.next();
            if (!m2) { return {}; }

            return item_type{*std::move(m1), *std::move(m2)};
        };
        return consume().adapt(std::move(fn));
    }

    constexpr auto enumerate() &&;

    template <std::size_t N>
    constexpr auto elements() &&
    {
        return consume().map(
            [] (auto&& val) -> decltype(std::get<N>(FLOW_FWD(val))) {
            return std::get<N>(FLOW_FWD(val));
        });
    }

    template <typename = Derived>
    constexpr auto keys() &&
    {
        return consume().template elements<0>();
    }

    template <typename = Derived>
    constexpr auto values() &&
    {
        return consume().template elements<1>();
    }

    template <typename = Derived>
    constexpr auto to_range() &&;

    template <typename C>
    constexpr auto to() &&
    {
        auto rng = consume().to_range();
        static_assert(std::is_constructible_v<C, decltype(rng.begin()), decltype(rng.end())>);
        return C(rng.begin(), rng.end());
    }

    template <template <typename...> typename C>
    constexpr auto to() &&
    {
        auto rng = consume().to_range();
        return C(rng.begin(), rng.end());
    }

    template <typename = Derived>
    constexpr auto collect() &&;

    constexpr auto to_vector() &&
    {
        return consume().template to<std::vector>();
    }

    template <typename T>
    constexpr auto to_vector() && -> std::vector<T>
    {
        return consume().template to<std::vector<T>>();
    }

    template <typename = Derived>
    constexpr auto to_string() &&
    {
        return consume().template to<std::string>();
    }

    template <typename Iter>
    constexpr auto output_to(Iter oiter) && -> Iter
    {
        consume().for_each([&oiter] (auto&& val) {
            *oiter = FLOW_FWD(val);
            ++oiter;
        });
        return oiter;
    }

    template <typename Sep = const char*>
    constexpr auto write_to(std::ostream& os, Sep sep = ", ") &&
    {
        consume().for_each([&os, &sep, first = true](auto&& m) mutable {
            if (first) {
                first = false;
            } else {
                os << sep;
            }
            os << FLOW_FWD(m);
        });
    }

    friend auto operator<<(std::ostream& os, flow_base&& self) -> std::ostream&
    {
        std::move(self).stream_to(os);
        return os;
    }
};

}

#endif