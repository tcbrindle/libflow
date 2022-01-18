
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLOW_HPP_INCLUDED
#define FLOW_HPP_INCLUDED

#include <flow/core/flow_base.hpp>
#include <flow/core/macros.hpp>
#include <flow/core/predicates.hpp>

#include <flow/op/all_any_none.hpp>
#include <flow/op/cartesian_product.hpp>
#include <flow/op/cartesian_product_with.hpp>
#include <flow/op/chain.hpp>
#include <flow/op/chunk.hpp>
#include <flow/op/collect.hpp>
#include <flow/op/contains.hpp>
#include <flow/op/count.hpp>
#include <flow/op/count_if.hpp>
#include <flow/op/cycle.hpp>
#include <flow/op/deref.hpp>
#include <flow/op/drop.hpp>
#include <flow/op/drop_while.hpp>
#include <flow/op/equal.hpp>
#include <flow/op/filter.hpp>
#include <flow/op/find.hpp>
#include <flow/op/flatten.hpp>
#include <flow/op/fold.hpp>
#include <flow/op/for_each.hpp>
#include <flow/op/group_by.hpp>
#include <flow/op/inspect.hpp>
#include <flow/op/interleave.hpp>
#include <flow/op/is_sorted.hpp>
#include <flow/op/map.hpp>
#include <flow/op/map_refinements.hpp>
#include <flow/op/minmax.hpp>
#include <flow/op/output_to.hpp>
#include <flow/op/product.hpp>
#include <flow/op/reverse.hpp>
#include <flow/op/scan.hpp>
#include <flow/op/slide.hpp>
#include <flow/op/split.hpp>
#include <flow/op/stride.hpp>
#include <flow/op/sum.hpp>
#include <flow/op/take.hpp>
#include <flow/op/take_while.hpp>
#include <flow/op/to.hpp>
#include <flow/op/to_range.hpp>
#include <flow/op/try_fold.hpp>
#include <flow/op/try_for_each.hpp>
#include <flow/op/write_to.hpp>
#include <flow/op/zip.hpp>
#include <flow/op/zip_with.hpp>

#include <flow/source/any_flow.hpp>
#include <flow/source/async.hpp>
#include <flow/source/c_str.hpp>
#include <flow/source/empty.hpp>
#include <flow/source/from.hpp>
#include <flow/source/generate.hpp>
#include <flow/source/iota.hpp>
#include <flow/source/istream.hpp>
#include <flow/source/istreambuf.hpp>
#include <flow/source/of.hpp>

#endif
