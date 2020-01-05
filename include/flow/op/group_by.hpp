
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
    bool group_exhausted_ = false;

    struct group : flow_base<group>
    {
        constexpr explicit group(group_by_adaptor& parent)
            : parent_(std::addressof(parent))
        {}

        constexpr group(group&&) noexcept = default;
        constexpr group& operator=(group&&) noexcept = default;

        constexpr auto next() -> next_t<Base>
        {
            if (parent_->group_exhausted_) {
                return {};
            }

            if (!parent_->next_item_) {
                if (!parent_->do_next()) {
                    parent_->group_exhausted_ = true;
                    return {};
                }
            }

            auto item = std::move(parent_->next_item_);
            parent_->next_item_.reset();

            return item;
        }

    private:
        group_by_adaptor* parent_;
    };

    constexpr auto do_next() -> bool
    {
        next_item_ = base_.next();
        if (!next_item_) {
            // We are done
            return false;
        }

        auto&& next_key = invoke(key_, *next_item_);
        if (next_key != *last_key_) {
            last_key_ = FLOW_FWD(next_key);
            return false;
        }

        return true;
    }

public:
    constexpr group_by_adaptor(Base&& base, Key&& key)
        : base_(std::move(base)), key_(std::move(key))
    {}

    constexpr auto next() -> maybe<group>
    {
        // Is this the first time we're being called?
        if (!last_key_) {
            next_item_ = base_.next();
            if (next_item_) {
                last_key_ = invoke(key_, *next_item_);
                return group{*this};
            } else {
                return {};
            }
        }

        // If the last group was exhausted, go ahead and start a new group
        if (group_exhausted_) {
            // Did we run out of items?
            if (!next_item_) {
                return {};
            }

            group_exhausted_ = false;
            return group{*this};
        }

        // Last group was not exhausted: fast-forward to the next group
        while (do_next())
            ;
        if (!next_item_) {
            return {};
        }
        return group{*this};

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
