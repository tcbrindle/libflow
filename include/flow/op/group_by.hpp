
#ifndef FLOW_OP_GROUP_BY_HPP_INCLUDED
#define FLOW_OP_GROUP_BY_HPP_INCLUDED

#include <flow/core/flow_base.hpp>

namespace flow {

namespace detail {

template <typename Base, typename Key>
struct group_by_adaptor : flow_base<group_by_adaptor<Base, Key>> {
private:

    using key_type = std::invoke_result_t<Key&, item_t<Base>&>;

    Base base_;
    FLOW_NO_UNIQUE_ADDRESS Key key_;
    next_t<Base> next_item_{};
    maybe<key_type> last_key_{};
    bool inner_exhausted_ = false;

    struct inner_flow : flow_base<inner_flow>
    {
        constexpr explicit inner_flow(group_by_adaptor& parent)
            : parent_(std::addressof(parent))
        {}

        constexpr auto next() -> next_t<Base>
        {
            if (done_) {
                parent_->inner_exhausted_ = true;
                return {};
            }

            auto item = std::move(parent_->next_item_);
            done_ = !parent_->do_next();

            return item;
        }

    private:
        group_by_adaptor* parent_;
        bool done_ = false;
    };

    constexpr auto do_next() -> bool
    {
        next_item_ = base_.next();
        if (!next_item_) {
            // We are done
            return false;
        }

        auto next_key = invoke(key_, *next_item_);
        bool ret = next_key == *last_key_;
        last_key_ = next_key;
        return ret;
    }

public:
    constexpr group_by_adaptor(Base&& base, Key&& key)
        : base_(std::move(base)), key_(std::move(key))
    {}

    constexpr auto next() -> maybe<inner_flow>
    {
        // Is this the first time we have been called?
        if (!next_item_) {
            next_item_ = base_.next();
            if (!next_item_) {
                return {};
            }
            last_key_ = invoke(key_, *next_item_);
            return inner_flow{*this};
        }

        // Last inner may not have been exhausted: fast-forward till
        // we find the start of the next group
        if (inner_exhausted_) {
            inner_exhausted_ = false;
        } else {
            while (true) {
                auto b = do_next();
                if (!next_item_) {
                    return {};
                }
                if (!b) {
                    break;
                }
            }
        }

        // We're good to go
        return inner_flow{*this};
    }
};

}

template <typename Derived>
template <typename Key>
constexpr auto flow_base<Derived>::group_by(Key key) &&
{
    return detail::group_by_adaptor<Derived, Key>(consume(), std::move(key));
}

}

#endif
