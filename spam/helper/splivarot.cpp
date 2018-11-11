#include "splivarot.h"
#include "geom.h"
#include <2geom/svg-path-parser.h>
#include <2geom/svg-path-writer.h>
#include <glib.h>

Path *Path_for_pathvector(Geom::PathVector const &epathv)
{
    /*std::cout << "converting to Livarot path" << std::endl;

    Geom::SVGPathWriter wr;
    wr.feed(epathv);
    std::cout << wr.str() << std::endl;*/

    Path *dest = new Path;
    dest->LoadPathVector(epathv);
    return dest;
}

Geom::PathVector sp_pathvector_boolop(Geom::PathVector const &pathva,
    Geom::PathVector const &pathvb,
    bool_op bop, fill_typ fra, fill_typ frb)
{
    // extract the livarot Paths from the source objects
    // also get the winding rule specified in the style
    int nbOriginaux = 2;
    std::vector<Path *> originaux(nbOriginaux);
    std::vector<FillRule> origWind(nbOriginaux);
    origWind[0] = fra;
    origWind[1] = frb;
    Geom::PathVector patht;
    // Livarot's outline of arcs is broken. So convert the path to linear and cubics only, for which the outline is created correctly. 
    originaux[0] = Path_for_pathvector(pathv_to_linear_and_cubic_beziers(pathva));
    originaux[1] = Path_for_pathvector(pathv_to_linear_and_cubic_beziers(pathvb));

    // some temporary instances, first
    Shape *theShapeA = new Shape;
    Shape *theShapeB = new Shape;
    Shape *theShape = new Shape;
    Path *res = new Path;
    res->SetBackData(false);
    Path::cut_position  *toCut = NULL;
    int                  nbToCut = 0;

    if (bop == bool_op_inters || bop == bool_op_union || bop == bool_op_diff || bop == bool_op_symdiff) {
        // true boolean op
        // get the polygons of each path, with the winding rule specified, and apply the operation iteratively
        originaux[0]->ConvertWithBackData(0.1);

        originaux[0]->Fill(theShape, 0);

        theShapeA->ConvertToShape(theShape, origWind[0]);

        originaux[1]->ConvertWithBackData(0.1);

        originaux[1]->Fill(theShape, 1);

        theShapeB->ConvertToShape(theShape, origWind[1]);

        theShape->Booleen(theShapeB, theShapeA, bop);

    }
    else if (bop == bool_op_cut) {
        // cuts= sort of a bastard boolean operation, thus not the axact same modus operandi
        // technically, the cut path is not necessarily a polygon (thus has no winding rule)
        // it is just uncrossed, and cleaned from duplicate edges and points
        // then it's fed to Booleen() which will uncross it against the other path
        // then comes the trick: each edge of the cut path is duplicated (one in each direction),
        // thus making a polygon. the weight of the edges of the cut are all 0, but
        // the Booleen need to invert the ones inside the source polygon (for the subsequent
        // ConvertToForme)

        // the cut path needs to have the highest pathID in the back data
        // that's how the Booleen() function knows it's an edge of the cut

        // FIXME: this gives poor results, the final paths are full of extraneous nodes. Decreasing
        // ConvertWithBackData parameter below simply increases the number of nodes, so for now I
        // left it at 1.0. Investigate replacing this by a combination of difference and
        // intersection of the same two paths. -- bb
        {
            Path* swap = originaux[0]; originaux[0] = originaux[1]; originaux[1] = swap;
            int   swai = origWind[0]; origWind[0] = origWind[1]; origWind[1] = (fill_typ)swai;
        }
        originaux[0]->ConvertWithBackData(1.0);

        originaux[0]->Fill(theShape, 0);

        theShapeA->ConvertToShape(theShape, origWind[0]);

        originaux[1]->ConvertWithBackData(1.0);

        originaux[1]->Fill(theShape, 1, false, false, false); //do not closeIfNeeded

        theShapeB->ConvertToShape(theShape, fill_justDont); // fill_justDont doesn't computes winding numbers

                                                            // les elements arrivent en ordre inverse dans la liste
        theShape->Booleen(theShapeB, theShapeA, bool_op_cut, 1);

    }
    else if (bop == bool_op_slice) {
        // slice is not really a boolean operation
        // you just put the 2 shapes in a single polygon, uncross it
        // the points where the degree is > 2 are intersections
        // just check it's an intersection on the path you want to cut, and keep it
        // the intersections you have found are then fed to ConvertPositionsToMoveTo() which will
        // make new subpath at each one of these positions
        // inversion pour l'opration
        {
            Path* swap = originaux[0]; originaux[0] = originaux[1]; originaux[1] = swap;
            int   swai = origWind[0]; origWind[0] = origWind[1]; origWind[1] = (fill_typ)swai;
        }
        originaux[0]->ConvertWithBackData(1.0);

        originaux[0]->Fill(theShapeA, 0, false, false, false); // don't closeIfNeeded

        originaux[1]->ConvertWithBackData(1.0);

        originaux[1]->Fill(theShapeA, 1, true, false, false);// don't closeIfNeeded and just dump in the shape, don't reset it

        theShape->ConvertToShape(theShapeA, fill_justDont);

        if (theShape->hasBackData()) {
            // should always be the case, but ya never know
            {
                for (int i = 0; i < theShape->numberOfPoints(); i++) {
                    if (theShape->getPoint(i).totalDegree() > 2) {
                        // possibly an intersection
                        // we need to check that at least one edge from the source path is incident to it
                        // before we declare it's an intersection
                        int cb = theShape->getPoint(i).incidentEdge[FIRST];
                        int   nbOrig = 0;
                        int   nbOther = 0;
                        int   piece = -1;
                        float t = 0.0;
                        while (cb >= 0 && cb < theShape->numberOfEdges()) {
                            if (theShape->ebData[cb].pathID == 0) {
                                // the source has an edge incident to the point, get its position on the path
                                piece = theShape->ebData[cb].pieceID;
                                if (theShape->getEdge(cb).st == i) {
                                    t = theShape->ebData[cb].tSt;
                                }
                                else {
                                    t = theShape->ebData[cb].tEn;
                                }
                                nbOrig++;
                            }
                            if (theShape->ebData[cb].pathID == 1) nbOther++; // the cut is incident to this point
                            cb = theShape->NextAt(i, cb);
                        }
                        if (nbOrig > 0 && nbOther > 0) {
                            // point incident to both path and cut: an intersection
                            // note that you only keep one position on the source; you could have degenerate
                            // cases where the source crosses itself at this point, and you wouyld miss an intersection
                            toCut = (Path::cut_position*)realloc(toCut, (nbToCut + 1) * sizeof(Path::cut_position));
                            toCut[nbToCut].piece = piece;
                            toCut[nbToCut].t = t;
                            nbToCut++;
                        }
                    }
                }
            }
            {
                // i think it's useless now
                int i = theShape->numberOfEdges() - 1;
                for (; i >= 0; i--) {
                    if (theShape->ebData[i].pathID == 1) {
                        theShape->SubEdge(i);
                    }
                }
            }

        }
    }

    int*    nesting = NULL;
    int*    conts = NULL;
    int     nbNest = 0;
    // pour compenser le swap juste avant
    if (bop == bool_op_slice) {
        //    theShape->ConvertToForme(res, nbOriginaux, originaux, true);
        //    res->ConvertForcedToMoveTo();
        res->Copy(originaux[0]);
        res->ConvertPositionsToMoveTo(nbToCut, toCut); // cut where you found intersections
        free(toCut);
    }
    else if (bop == bool_op_cut) {
        // il faut appeler pour desallouer PointData (pas vital, mais bon)
        // the Booleen() function did not deallocate the point_data array in theShape, because this
        // function needs it.
        // this function uses the point_data to get the winding number of each path (ie: is a hole or not)
        // for later reconstruction in objects, you also need to extract which path is parent of holes (nesting info)
        theShape->ConvertToFormeNested(res, nbOriginaux, &originaux[0], 1, nbNest, nesting, conts);
    }
    else {
        theShape->ConvertToForme(res, nbOriginaux, &originaux[0]);
    }

    delete theShape;
    delete theShapeA;
    delete theShapeB;
    delete originaux[0];
    delete originaux[1];

    gchar *result_str = res->svg_dump_path();
    Geom::PathVector outres = Geom::parse_svg_path(result_str);
    g_free(result_str);

    delete res;
    return outres;
}
