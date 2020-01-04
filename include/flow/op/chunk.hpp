
#ifndef FLOW_OP_CHUNK_HPP_INCLUDED
#define FLOW_OP_CHUNK_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Base>
struct chunk_adaptor : flow_base<chunk_adaptor<Base>> {
private:
    Base base_;
    const dist_t size_;
    dist_t counter_{};
    next_t<Base> next_item_{};

    struct inner_flow : flow_base<inner_flow> {
        constexpr explicit inner_flow(chunk_adaptor& parent)
            : parent_(&parent)
        {}

        constexpr inner_flow(inner_flow&&) noexcept = default;
        constexpr inner_flow& operator=(inner_flow&&) noexcept = default;

        constexpr auto next() -> next_t<Base>
        {
             if (parent_->counter_ == 0) {
                 return {};
             }

             --parent_->counter_;

             auto m = std::move(parent_->next_item_);
             parent_->next_item_ = parent_->base_.next();

             return m;
        }

    private:
        chunk_adaptor* parent_;
    };

public:
    constexpr explicit chunk_adaptor(Base&& base, dist_t size)
        : base_(std::move(base)),
          size_(size)
    {}

    constexpr auto next() -> maybe<inner_flow>
    {
        if (counter_ > 0) {
            auto m = base_.advance(counter_);
            counter_ = 0;
            if (!m) {
                return {};
            }
        }

        if (!next_item_) {
            next_item_ = base_.next();
            if (!next_item_) {
                return {};
            }
        }

        counter_ = size_;
        return {inner_flow{*this}};
    }
};

}

template <typename Derived>
constexpr auto flow_base<Derived>::chunk(dist_t size) &&
{
    assert((size > 0) && "Chunk size must be greater than zero");
    return detail::chunk_adaptor<Derived>{consume(), size};
}

}

#endif
