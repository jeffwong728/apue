#ifndef SEEN_SP_STYLE_H
#define SEEN_SP_STYLE_H

/** \file
 * SPStyle - a style object for SPItem objects
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 2014 Tavmjong Bah
 * Copyright (C) 2010 Jon A. Cruz
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

/// An SVG style object.
class SPStyle {

public:
    struct {
        std::vector<double> values;
    } stroke_dasharray;

    struct {
        double value;
    } stroke_dashoffset;
};

SPStyle *sp_style_ref(SPStyle *style); // SPStyle::ref();

SPStyle *sp_style_unref(SPStyle *style); // SPStyle::unref();

void sp_style_set_to_uri_string (SPStyle *style, bool isfill, const char *uri); // ?

char const *sp_style_get_css_unit_string(int unit);  // No change?

#define SP_CSS_FONT_SIZE_DEFAULT 12.0
double sp_style_css_size_px_to_units(double size, int unit, double font_size = SP_CSS_FONT_SIZE_DEFAULT); // No change?
double sp_style_css_size_units_to_px(double size, int unit, double font_size = SP_CSS_FONT_SIZE_DEFAULT); // No change?

#endif // SEEN_SP_STYLE_H


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
 
