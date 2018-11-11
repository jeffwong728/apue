/*
 * Multiindex container for selection
 *
 * Authors:
 *   Adrian Boguszewski
 *   Marc Jeanmougin
 *
 * Copyright (C) 2016 Adrian Boguszewski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_PROTOTYPE_OBJECTSET_H
#define INKSCAPE_PROTOTYPE_OBJECTSET_H

#include <string>
#include <unordered_map>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/sub_range.hpp>
#include <boost/range/any_range.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

enum BoolOpErrors {
    DONE,
    DONE_NO_PATH,
    DONE_NO_ACTION,
    ERR_TOO_LESS_PATHS_1,
    ERR_TOO_LESS_PATHS_2,
    ERR_NO_PATHS,
    ERR_Z_ORDER
};

// boolean operation
enum bool_op
{
  bool_op_union,		// A OR B
  bool_op_inters,		// A AND B
  bool_op_diff,			// A \ B
  bool_op_symdiff,  // A XOR B
  bool_op_cut,      // coupure (pleines)
  bool_op_slice     // coupure (contour)
};
typedef enum bool_op BooleanOp;

#endif //INKSCAPE_PROTOTYPE_OBJECTSET_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
