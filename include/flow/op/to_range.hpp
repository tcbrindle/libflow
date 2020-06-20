
#ifndef FLOW_OP_TO_RANGE_HPP_INCLUDED
#define FLOW_OP_TO_RANGE_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename F>
struct flow_range {
private:
    struct iterator {
        using value_type = value_t<F>;
        using reference = item_t<F>;
        using difference_type = dist_t;
        using pointer = value_type*;
        using iterator_category = std::input_iterator_tag;

        constexpr iterator() = default;

        constexpr explicit iterator(flow_range& parent)
            : parent_(std::addressof(parent))
        {}

        constexpr iterator& operator++()
        {
            if (parent_) {
                if (!parent_->do_next()) {
                    parent_ = nullptr;
                }
            }
            return *this;
        }

        constexpr reference operator*() const
        {
            return *std::move(parent_->item_);
        }

        friend constexpr bool operator==(const iterator& lhs,
                                         const iterator& rhs)
        {
            return lhs.parent_ == rhs.parent_;
        }

        friend constexpr bool operator!=(const iterator& lhs,
                                         const iterator& rhs)
        {
            return !(lhs == rhs);
        }

    private:
        flow_range* parent_ = nullptr;
    };

public:
    constexpr explicit flow_range(F&& flow)
        : flow_(std::move(flow)), item_(flow_.next())
    {}

    constexpr auto begin() { return iterator{*this}; }
    constexpr auto end() { return iterator{}; }

private:
    bool do_next()
    {
        item_ = flow_.next();
        return (bool) item_;
    }

    F flow_;
    maybe<item_t<F>> item_;
};

}

template <typename D>
constexpr auto flow_base<D>::to_range() &&
{
    return detail::flow_range<D>{consume()};
}

}

#endif