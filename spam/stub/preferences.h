/** @file
 * Singleton class to access the preferences file in a convenient way.
 */
/* Authors:
 *   Krzysztof Kosi_ski <tweenk.pl@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2008,2009 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_PREFSTORE_H
#define INKSCAPE_PREFSTORE_H

#include <climits>
#include <cfloat>
#include <glibmm/ustring.h>
#include <algorithm>

namespace Inkscape {
class CMSSystem;
class Preferences
{
public:
    /**
     * Access the singleton Preferences object.
     */
    inline static Preferences *get();
    int getInt(Glib::ustring const &, int def = 0) { return def; }
    bool getBool(Glib::ustring const &, bool def = false) { return def; }
    int getIntLimited(Glib::ustring const &, int def = 0, int min = INT_MIN, int max = INT_MAX) { return def; }
};

static Preferences s_Inkscape_Preferences_instance;
inline Preferences *Preferences::get() { return &s_Inkscape_Preferences_instance; }
} // namespace Inkscape

#endif // INKSCAPE_PREFSTORE_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:75
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
