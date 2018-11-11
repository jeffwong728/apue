#ifndef SPAM_HELPER_SP_LIVAROT_H
#define SPAM_HELPER_SP_LIVAROT_H

#include <2geom/forward.h>
#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <livarot/Path.h>
#include <livarot/Shape.h>

Path *Path_for_pathvector(Geom::PathVector const &epathv);
Geom::PathVector sp_pathvector_boolop(Geom::PathVector const &pathva, Geom::PathVector const &pathvb, bool_op bop, fill_typ fra, fill_typ frb);

#endif  // SPAM_HELPER_SP_LIVAROT_H
