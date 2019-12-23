
#ifndef FLOW_SOURCE_GENERATE_HPP_INCLUDED
#define FLOW_SOURCE_GENERATE_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

inline constexpr struct {
private:
    template <typename Func>
    struct generator : flow_base<generator<Func>> {

        constexpr explicit generator(Func func)
            : func_(std::move(func))
        {}

        constexpr auto next() -> maybe<std::invoke_result_t<Func&>>
        {
            return func_();
        }

    private:
        FLOW_NO_UNIQUE_ADDRESS Func func_;
    };

public:
    template <typename Func>
    constexpr auto operator()(Func func) const
    {
        static_assert(std::is_invocable_v<Func&>);
        return generator<Func>{std::move(func)};
    }

} generate;

}

#endif
