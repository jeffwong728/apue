#include "geomnode.h"
#include <ui/evts.h>
#include <ui/projs/stationnode.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/2geom.h>
#include <2geom/path-intersection.h>
#pragma warning( pop )

GeomNode::GeomNode(const SPModelNode &parent, const wxString &title)
    : DrawableNode(parent, title)
{
}

GeomNode::~GeomNode()
{
}

SPSpamRgn GeomNode::ToRegion() const
{
    SPSpamRgn rgn = std::make_shared<SpamRgn>();
    Geom::OptRect bbox = GetBoundingBox();
    Geom::PathVector pv;
    BuildPath(pv);

    if (bbox)
    {
        int t = wxRound(bbox.get().top());
        int b = wxRound(bbox.get().bottom());
        int l = wxRound(bbox.get().left());
        int r = wxRound(bbox.get().right());
        for (int l = t; l<b; ++l)
        {
            int cb = -1;
            for (int c = l; c < r; ++c)
            {
                if (IsPointInside(pv, Geom::Point(c, l)))
                {
                    if (cb < 0)
                    {
                        cb = c;
                    }
                }
                else
                {
                    if (cb > 0)
                    {
                        rgn->AddRun(l, cb, c);
                        cb = -1;
                    }
                }
            }

            if (cb > 0)
            {
                rgn->AddRun(l, cb, r);
            }
        }
    }

    return rgn;
}

EntitySigType GeomNode::GetCreateSigType() const
{
    return EntitySigType::kGeomCreate;
}

EntitySigType GeomNode::GetAddSigType() const
{
    return EntitySigType::kGeomAdd;
}

EntitySigType GeomNode::GetDeleteSigType() const
{
    return EntitySigType::kGeomDelete;
}

bool GeomNode::IsPointInside(Geom::PathVector &pv, const Geom::Point &pt)
{
    if (!pv.empty())
    {
        int numWinding = 0;
        for (const Geom::Path &pth : pv)
        {
            if (Geom::contains(pth, pt))
            {
                numWinding += 1;
            }
        }

        if (numWinding & 1)
        {
            return true;
        }
    }

    return false;
}