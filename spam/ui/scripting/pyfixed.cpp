#include "pyfixed.h"
#include <ui/projs/rectnode.h>
#include <ui/toplevel/rootframe.h>
#include <ui/projs/fixednode.h>
#include <ui/projs/stationnode.h>
#include <ui/cv/cairocanvas.h>

PyFixed::PyFixed()
{
}

bool PyFixed::SetFeatureImpl(const std::string &feature)
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spFixed = std::dynamic_pointer_cast<FixedNode>(spObj);
        if (spFixed)
        {
            if (feature == "area")
            {
                spFixed->SetFeature(RegionFeatureFlag::kRFF_AREA);
            }
            else if (feature == "length")
            {
                spFixed->SetFeature(RegionFeatureFlag::kRFF_LENGTH);
            }
            else if (feature == "orientation")
            {
                spFixed->SetFeature(RegionFeatureFlag::kRFF_ORIENTATION);
            }
            else if (feature == "diameter")
            {
                spFixed->SetFeature(RegionFeatureFlag::kRFF_DIAMETER);
            }
            else if (feature == "smallest_circle")
            {
                spFixed->SetFeature(RegionFeatureFlag::kRFF_SMALLEST_CIRCLE);
            }
            else if (feature == "convex_hull")
            {
                spFixed->SetFeature(RegionFeatureFlag::kRFF_CONVEX_HULL);
            }
            else if (feature == "elliptic_axis")
            {
                spFixed->SetFeature(RegionFeatureFlag::kRFF_ELLIPTIC_AXIS);
            }
            else if (feature == "rect2")
            {
                spFixed->SetFeature(RegionFeatureFlag::kRFF_RECT2);
            }
            else
            {
                throw std::invalid_argument(std::string("Invalid feature: ") + feature);
            }

            return true;
        }
    }

    return false;
}

bool PyFixed::ClearFeatureImpl(const std::string &feature)
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spFixed = std::dynamic_pointer_cast<FixedNode>(spObj);
        if (spFixed)
        {
            if (feature.empty())
            {
                spFixed->ClearAllFeatures();
            }
            else if (feature == "area")
            {
                spFixed->ClearFeature(RegionFeatureFlag::kRFF_AREA);
            }
            else if (feature == "length")
            {
                spFixed->ClearFeature(RegionFeatureFlag::kRFF_LENGTH);
            }
            else if (feature == "orientation")
            {
                spFixed->ClearFeature(RegionFeatureFlag::kRFF_ORIENTATION);
            }
            else if (feature == "diameter")
            {
                spFixed->ClearFeature(RegionFeatureFlag::kRFF_DIAMETER);
            }
            else if (feature == "smallest_circle")
            {
                spFixed->ClearFeature(RegionFeatureFlag::kRFF_SMALLEST_CIRCLE);
            }
            else if (feature == "convex_hull")
            {
                spFixed->ClearFeature(RegionFeatureFlag::kRFF_CONVEX_HULL);
            }
            else if (feature == "elliptic_axis")
            {
                spFixed->ClearFeature(RegionFeatureFlag::kRFF_ELLIPTIC_AXIS);
            }
            else if (feature == "rect2")
            {
                spFixed->ClearFeature(RegionFeatureFlag::kRFF_RECT2);
            }
            else
            {
                throw std::invalid_argument(std::string("Invalid feature: ") + feature);
            }

            return true;
        }
    }

    return false;
}

void PyFixed::SetFeature(const std::string &feature)
{
    Geom::OptRect oldBBox;
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spFixed = std::dynamic_pointer_cast<FixedNode>(spObj);
        if (spFixed)
        {
            oldBBox = spFixed->GetBoundingBox();
        }
    }

    if (SetFeatureImpl(feature))
    {
        auto spObj = wpObj.lock();
        if (spObj)
        {
            auto spFixed = std::dynamic_pointer_cast<FixedNode>(spObj);
            auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
            auto spStation = std::dynamic_pointer_cast<StationNode>(spFixed->GetParent());
            if (spStation && frame)
            {
                CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
                if (cavs)
                {
                    oldBBox.unionWith(spFixed->GetBoundingBox());
                    cavs->RefreshRect(oldBBox);
                    ::wxYield();
                }
            }
        }
    }
}

void PyFixed::SetFeature(const std::vector<std::string> &features)
{
    Geom::OptRect oldBBox;
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spFixed = std::dynamic_pointer_cast<FixedNode>(spObj);
        if (spFixed)
        {
            oldBBox = spFixed->GetBoundingBox();
        }
    }

    bool bSuccess = !features.empty();
    for (const std::string &feature : features)
    {
        bSuccess = bSuccess && SetFeatureImpl(feature);
    }

    if (bSuccess)
    {
        auto spObj = wpObj.lock();
        if (spObj)
        {
            auto spFixed = std::dynamic_pointer_cast<FixedNode>(spObj);
            auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
            auto spStation = std::dynamic_pointer_cast<StationNode>(spFixed->GetParent());
            if (spStation && frame)
            {
                CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
                if (cavs)
                {
                    oldBBox.unionWith(spFixed->GetBoundingBox());
                    cavs->RefreshRect(oldBBox);
                    ::wxYield();
                }
            }
        }
    }
}

void PyFixed::ClearFeature(const std::string &feature)
{
    Geom::OptRect oldBBox;
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spFixed = std::dynamic_pointer_cast<FixedNode>(spObj);
        if (spFixed)
        {
            oldBBox = spFixed->GetBoundingBox();
        }
    }

    if (ClearFeatureImpl(feature))
    {
        auto spObj = wpObj.lock();
        if (spObj)
        {
            auto spFixed = std::dynamic_pointer_cast<FixedNode>(spObj);
            auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
            auto spStation = std::dynamic_pointer_cast<StationNode>(spFixed->GetParent());
            if (spStation && frame)
            {
                CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
                if (cavs)
                {
                    oldBBox.unionWith(spFixed->GetBoundingBox());
                    cavs->RefreshRect(oldBBox);
                    ::wxYield();
                }
            }
        }
    }
}

void PyFixed::ClearFeature(const std::vector<std::string> &features)
{
    if (features.empty())
    {
        ClearFeature(std::string());
    }
    else
    {
        Geom::OptRect oldBBox;
        auto spObj = wpObj.lock();
        if (spObj)
        {
            auto spFixed = std::dynamic_pointer_cast<FixedNode>(spObj);
            if (spFixed)
            {
                oldBBox = spFixed->GetBoundingBox();
            }
        }

        bool bSuccess = true;
        for (const std::string &feature : features)
        {
            bSuccess = bSuccess && ClearFeatureImpl(feature);
        }

        if (bSuccess)
        {
            auto spObj = wpObj.lock();
            if (spObj)
            {
                auto spFixed = std::dynamic_pointer_cast<FixedNode>(spObj);
                auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
                auto spStation = std::dynamic_pointer_cast<StationNode>(spFixed->GetParent());
                if (spStation && frame)
                {
                    CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
                    if (cavs)
                    {
                        oldBBox.unionWith(spFixed->GetBoundingBox());
                        cavs->RefreshRect(oldBBox);
                        ::wxYield();
                    }
                }
            }
        }
    }
}

void PyFixed::ClearFeature()
{
    ClearFeature(std::string());
}

std::string PyFixed::toString() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spFixed = std::dynamic_pointer_cast<FixedNode>(spObj);
        if (spFixed)
        {
            return spFixed->GetTitle().ToStdString();
        }
    }

    return "Invalid FixedDrawable object";
}

void PyExportFixed(pybind11::module_ &m)
{
    auto c = pybind11::class_<PyFixed, PyDrawable>(m, "FixedDrawable");
    c.def("SetFeature", pybind11::overload_cast<const std::string &>(&PyFixed::SetFeature), "Set a display feature", pybind11::arg("feature"));
    c.def("SetFeature", pybind11::overload_cast<const std::vector<std::string> &>(&PyFixed::SetFeature), "Set a bunch of display features", pybind11::arg("features"));
    c.def("ClearFeature", pybind11::overload_cast<const std::string &>(&PyFixed::ClearFeature), "Clear a display feature", pybind11::arg("feature"));
    c.def("ClearFeature", pybind11::overload_cast<const std::vector<std::string> &>(&PyFixed::ClearFeature), "Clear a bunch of display features", pybind11::arg("features"));
    c.def("ClearFeature", pybind11::overload_cast<>(&PyFixed::ClearFeature), "Clear all display features");
    c.def("__repr__", &PyFixed::toString);
}
