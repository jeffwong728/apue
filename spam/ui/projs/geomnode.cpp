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

cv::Ptr<cv::mvlab::Region> GeomNode::ToRegion() const
{
    cv::Ptr<cv::mvlab::Region> rgn = cv::mvlab::Region::GenEmpty();
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