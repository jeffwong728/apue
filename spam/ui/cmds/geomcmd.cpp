#include "geomcmd.h"
#include <ui/spam.h>
#include <ui/projs/stationnode.h>
#include <ui/projs/geomnode.h>
#include <ui/projs/rectnode.h>
#include <ui/projs/beziergonnode.h>
#include <ui/projs/projtreemodel.h>
#include <helper/splivarot.h>
#include <2geom/svg-path-parser.h>
#include <2geom/svg-path-writer.h>
#ifdef realloc
#undef realloc
#endif
//#include <SkString.h>
//#include <SkParsePath.h>
//#include <SkPathOps.h>

GeomCmd::GeomCmd(ProjTreeModel *model, SPStationNode &station)
    : SpamCmd()
    , model_(model)
    , station_(station)
{
}

GeomCmd::GeomCmd(ProjTreeModel *model, SPStationNode &station, SPGeomNode &geom)
    : SpamCmd()
    , model_(model)
    , station_(station)
    , geom_(geom)
{
}

DeleteGeomsCmd::DeleteGeomsCmd(ProjTreeModel *model, const SPGeomNodeVector &geoms)
    : SpamCmd()
    , model_(model)
{
    for (const auto &geom : geoms)
    {
        auto station = std::dynamic_pointer_cast<StationNode>(geom->GetParent());
        if (station)
        {
            auto &geomData = geoms_[station->GetUUIDTag()];
            geomData.first = station;
            geomData.second.push_back(geom);
        }
    }
}

void DeleteGeomsCmd::Do()
{
    if (model_ && !geoms_.empty())
    {
        wxDataViewItemArray geoms;
        for (const auto &geomItem : geoms_)
        {
            for (const auto &geom : geomItem.second.second)
            {
                geoms.push_back(wxDataViewItem(geom.get()));
            }
        }

        if (!geoms.empty())
        {
            model_->Delete(geoms, true);
        }
    }
}

void DeleteGeomsCmd::Undo()
{
    if (model_)
    {
        for (auto &geomItem : geoms_)
        {
            model_->AddToStations(geomItem.second.first, geomItem.second.second, true);
        }
    }
}

void DeleteGeomsCmd::Redo()
{
    Do();
}

wxString DeleteGeomsCmd::GetDescription() const
{
    return wxString(wxT("Delete "));
}

CreateRectCmd::CreateRectCmd(ProjTreeModel *model, SPStationNode &station, const wxString &wouldTitle, const RectData &data)
    : GeomCmd(model, station)
    , wouldTitle_(wouldTitle)
    , data_(data)
{
}

void CreateRectCmd::Do()
{
    if (model_ && station_)
    {
        geom_ = model_->CreateToStation(station_, data_);
    }
}

void CreateRectCmd::Undo()
{
    if (model_ && geom_)
    {
        model_->Delete(wxDataViewItem(geom_.get()), true);
    }
}

void CreateRectCmd::Redo()
{
    if (model_ && station_ && geom_)
    {
        model_->AddToStation(station_, geom_, true);
    }
}

wxString CreateRectCmd::GetDescription() const
{
    return wxString(wxT("Create rectangle ") + geom_->GetTitle());
}

CreateLineCmd::CreateLineCmd(ProjTreeModel *model, SPStationNode &station, const wxString &wouldTitle, const LineData &data)
    : GeomCmd(model, station)
    , wouldTitle_(wouldTitle)
    , data_(data)
{
}

void CreateLineCmd::Do()
{
    if (model_ && station_)
    {
        geom_ = model_->CreateToStation(station_, data_);
    }
}

void CreateLineCmd::Undo()
{
    if (model_ && geom_)
    {
        model_->Delete(wxDataViewItem(geom_.get()), true);
    }
}

void CreateLineCmd::Redo()
{
    if (model_ && station_ && geom_)
    {
        model_->AddToStation(station_, geom_, true);
    }
}

wxString CreateLineCmd::GetDescription() const
{
    return wxString(wxT("Create rectangle ") + geom_->GetTitle());
}

CreatePolygonCmd::CreatePolygonCmd(ProjTreeModel *model, SPStationNode &station, const wxString &wouldTitle, const PolygonData &data)
    : GeomCmd(model, station)
    , wouldTitle_(wouldTitle)
    , data_(data)
{
}

void CreatePolygonCmd::Do()
{
    if (model_ && station_)
    {
        geom_ = model_->CreateToStation(station_, data_);
    }
}

void CreatePolygonCmd::Undo()
{
    if (model_ && geom_)
    {
        model_->Delete(wxDataViewItem(geom_.get()), true);
    }
}

void CreatePolygonCmd::Redo()
{
    if (model_ && station_ && geom_)
    {
        model_->AddToStation(station_, geom_, true);
    }
}

wxString CreatePolygonCmd::GetDescription() const
{
    return wxString(wxT("Create polygon ") + geom_->GetTitle());
}

CreateEllipseCmd::CreateEllipseCmd(ProjTreeModel *model, SPStationNode &station, const wxString &wouldTitle, const GenericEllipseArcData &data)
    : GeomCmd(model, station)
    , wouldTitle_(wouldTitle)
    , data_(data)
{
}

void CreateEllipseCmd::Do()
{
    if (model_ && station_)
    {
        geom_ = model_->CreateToStation(station_, data_);
    }
}

void CreateEllipseCmd::Undo()
{
    if (model_ && geom_)
    {
        model_->Delete(wxDataViewItem(geom_.get()), true);
    }
}

void CreateEllipseCmd::Redo()
{
    if (model_ && station_ && geom_)
    {
        model_->AddToStation(station_, geom_, true);
    }
}

wxString CreateEllipseCmd::GetDescription() const
{
    return wxString(wxT("Create ellipse ") + geom_->GetTitle());
}

CreateBeziergonCmd::CreateBeziergonCmd(ProjTreeModel *model, SPStationNode &station, const wxString &wouldTitle, const BezierData &data)
    : GeomCmd(model, station)
    , wouldTitle_(wouldTitle)
    , data_(data)
{
}

void CreateBeziergonCmd::Do()
{
    if (model_ && station_)
    {
        geom_ = model_->CreateToStation(station_, data_);
    }
}

void CreateBeziergonCmd::Undo()
{
    if (model_ && geom_)
    {
        model_->Delete(wxDataViewItem(geom_.get()), true);
    }
}

void CreateBeziergonCmd::Redo()
{
    if (model_ && station_ && geom_)
    {
        model_->AddToStation(station_, geom_, true);
    }
}

wxString CreateBeziergonCmd::GetDescription() const
{
    return wxString(wxT("Create beziergon ") + geom_->GetTitle());
}

BoolCmd::BoolCmd(ProjTreeModel *model, const SPGeomNodeVector &geoms, const wxString &wouldTitle)
    : SpamCmd()
    , wouldTitle_(wouldTitle)
    , model_(model)
    , geoms_(geoms)
    , station_(std::dynamic_pointer_cast<StationNode>(geoms.front()->GetParent()))
{
}

void BoolCmd::Undo()
{
    if (model_)
    {
        model_->Delete(wxDataViewItem(uGeom_.get()), true);
        model_->AddToStations(station_, geoms_, true);
    }
}

void BoolCmd::Redo()
{
    if (model_ && !geoms_.empty())
    {
        wxDataViewItemArray geoms;
        for (const auto &g : geoms_)
        {
            geoms.push_back(wxDataViewItem(g.get()));
        }

        if (!geoms.empty())
        {
            model_->Delete(geoms, true);
        }

        model_->AddToStation(station_, uGeom_, true);
    }
}

void BoolCmd::BoolOp(const int op)
{
    if (model_ && !geoms_.empty())
    {
        Geom::PathVector upv;
        wxDataViewItemArray geoms;
        for (const auto &g : geoms_)
        {
            if (upv.empty())
            {
                g->BuildPath(upv);
            }
            else
            {
                Geom::PathVector pv;
                g->BuildPath(pv);

                std::string strpv = Geom::write_svg_path(pv);
                std::string strupv = Geom::write_svg_path(upv);
                //SkPath skpv, skupv;
                //if (SkParsePath::FromSVGString(strpv.c_str(), &skpv) && SkParsePath::FromSVGString(strupv.c_str(), &skupv))
                //{
                //    SkPath skResPath;
                //    if (Op(skpv, skupv, (op == UnionOp) ? kUnion_SkPathOp : kIntersect_SkPathOp, &skResPath))
                //    {
                //        SkString skStr;
                //        SkParsePath::ToSVGString(skResPath, &skStr);
                //        upv = Geom::parse_svg_path(skStr.c_str());
                //    }
                //    else
                //    {
                //        upv = sp_pathvector_boolop(pv, upv, (op == UnionOp) ? bool_op_union : bool_op_inters, fill_nonZero, fill_nonZero);
                //    }
                //}
                //else
                {
                    upv = sp_pathvector_boolop(pv, upv, (op == UnionOp) ? bool_op_union : bool_op_inters, fill_nonZero, fill_nonZero);
                }
            }

            geoms.push_back(wxDataViewItem(g.get()));
        }

        uGeom_ = std::make_shared<BeziergonNode>(station_, wouldTitle_, upv);
        uGeom_->drawStyle_.strokeWidth_ = SpamConfig::Get<int>(cp_ToolGeomStrokeWidth, 1);
        uGeom_->drawStyle_.strokeColor_.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomStrokePaint, wxBLUE->GetRGBA()));
        uGeom_->drawStyle_.fillColor_.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomFillPaint, wxYELLOW->GetRGBA()));

        if (!geoms.empty())
        {
            model_->Delete(geoms, true);
        }

        model_->AddToStation(station_, uGeom_, true);
    }
}

wxString UnionGeomsCmd::GetDescription() const
{
    return wxString(wxT("Union geometries"));
}

wxString IntersectionGeomsCmd::GetDescription() const
{
    return wxString(wxT("Intersection geometries"));
}

BinaryBoolGeomsCmd::BinaryBoolGeomsCmd(ProjTreeModel *model, const SPGeomNode &geom1, const SPGeomNode &geom2, const wxString &wouldTitle)
    : SpamCmd()
    , wouldTitle_(wouldTitle)
    , model_(model)
    , geom1_(geom1)
    , geom2_(geom2)
    , station_(std::dynamic_pointer_cast<StationNode>(geom1->GetParent()))
{
}

void BinaryBoolGeomsCmd::Undo()
{
    if (model_)
    {
        model_->Delete(wxDataViewItem(dGeom_.get()), true);
        SPGeomNodeVector geoms{ geom1_ , geom2_ };
        model_->AddToStations(station_, geoms, true);
    }
}

void BinaryBoolGeomsCmd::Redo()
{
    if (model_ && geom1_ && geom2_)
    {
        wxDataViewItemArray geoms;
        geoms.push_back(wxDataViewItem(geom1_.get()));
        geoms.push_back(wxDataViewItem(geom2_.get()));

        if (!geoms.empty())
        {
            model_->Delete(geoms, true);
        }

        model_->AddToStation(station_, dGeom_, true);
    }
}

void BinaryBoolGeomsCmd::BoolOp(const int op)
{
    if (model_ && geom1_ && geom2_)
    {
        Geom::PathVector pv1, pv2;
        geom1_->BuildPath(pv1);
        geom2_->BuildPath(pv2);
        Geom::PathVector dpv = sp_pathvector_boolop(pv2, pv1, (op == DiffOp) ? bool_op_diff : bool_op_symdiff, fill_nonZero, fill_nonZero);

        wxDataViewItemArray geoms;
        geoms.push_back(wxDataViewItem(geom1_.get()));
        geoms.push_back(wxDataViewItem(geom2_.get()));

        dGeom_ = std::make_shared<BeziergonNode>(station_, wouldTitle_, dpv);
        dGeom_->drawStyle_.strokeWidth_ = SpamConfig::Get<int>(cp_ToolGeomStrokeWidth, 1);
        dGeom_->drawStyle_.strokeColor_.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomStrokePaint, wxBLUE->GetRGBA()));
        dGeom_->drawStyle_.fillColor_.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomFillPaint, wxYELLOW->GetRGBA()));

        if (!geoms.empty())
        {
            model_->Delete(geoms, true);
        }

        model_->AddToStation(station_, dGeom_, true);
    }
}

wxString DiffGeomsCmd::GetDescription() const
{
    return wxString(wxT("Difference geometries"));
}

wxString XORGeomsCmd::GetDescription() const
{
    return wxString(wxT("Exclusive OR geometries"));
}