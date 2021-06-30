#include "proctool.h"
#include <wx/log.h>
#include <ui/spam.h>
#include <ui/toplevel/rootframe.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/drawablenode.h>

void FilterTool::OnOptionChanged(const EvToolOption &e)
{
    const int toolId = boost::get<int>(e.toolOptions.at(cp_ToolId));
    if (kSpamID_TOOLBOX_PROC_FILTER == toolId)
    {
        toolOptions = e.toolOptions;
        BoxToolImpl::ResetTool();
    }
}

void FilterTool::OnBoxingEnded(const EvBoxingEnded &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.mData.GetEventObject());
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (cav && frame)
    {
        std::map<std::string, int> iParams;
        std::map<std::string, double> fParams;
        BuildParameters(iParams, fParams);
        cav->UpdateFilter(*(e.boxRect), iParams, fParams);
        uuids.insert(cav->GetUUID());
    }
}

void FilterTool::OnImageClicked(const EvImageClicked &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (cav && frame)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        std::map<std::string, int> iParams;
        std::map<std::string, double> fParams;
        BuildParameters(iParams, fParams);
        cav->UpdateFilter(Geom::Rect(imgPt.x, imgPt.y, imgPt.x + 0.5, imgPt.y + 0.5), iParams, fParams);
        uuids.insert(cav->GetUUID());
    }
}

void FilterTool::OnEntityClicked(const EvEntityClicked &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.e.GetEventObject());
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (cav && frame)
    {
        Geom::PathVector pv;
        e.ent->BuildPath(pv);
        auto bbox = e.ent->GetBoundingBox();
        std::map<std::string, int> iParams;
        std::map<std::string, double> fParams;
        BuildParameters(iParams, fParams);
        cav->UpdateFilter(bbox ? *bbox : Geom::Rect(), iParams, fParams);
        uuids.insert(cav->GetUUID());
    }
}

sc::result FilterTool::react(const EvToolQuit &e)
{
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (frame)
    {
        for (const auto &uuid : uuids)
        {
            CairoCanvas *cav = frame->FindCanvasByUUID(uuid);
            if (cav)
            {
                cav->RemoveImageProcessData();
            }
        }
    }

    BoxToolT::QuitTool(e);
    return transit<NoTool>();
}

void FilterTool::BuildParameters(std::map<std::string, int> &iParams, std::map<std::string, double> &fParams)
{
    iParams[cp_ToolProcFilterType]                  = boost::get<int>(toolOptions[cp_ToolProcFilterType]);
    iParams[cp_ToolProcFilterBorderType]            = boost::get<int>(toolOptions[cp_ToolProcFilterBorderType]);
    iParams[cp_ToolProcFilterBoxKernelWidth]        = boost::get<int>(toolOptions[cp_ToolProcFilterBoxKernelWidth]);
    iParams[cp_ToolProcFilterBoxKernelHeight]       = boost::get<int>(toolOptions[cp_ToolProcFilterBoxKernelHeight]);
    iParams[cp_ToolProcFilterGaussianKernelWidth]   = boost::get<int>(toolOptions[cp_ToolProcFilterGaussianKernelWidth]);
    iParams[cp_ToolProcFilterGaussianKernelHeight]  = boost::get<int>(toolOptions[cp_ToolProcFilterGaussianKernelHeight]);
    iParams[cp_ToolProcFilterMedianKernelWidth]     = boost::get<int>(toolOptions[cp_ToolProcFilterMedianKernelWidth]);
    iParams[cp_ToolProcFilterMedianKernelHeight]    = boost::get<int>(toolOptions[cp_ToolProcFilterMedianKernelHeight]);
    iParams[cp_ToolProcFilterBilateralDiameter]     = boost::get<int>(toolOptions[cp_ToolProcFilterBilateralDiameter]);
    fParams[cp_ToolProcFilterGaussianSigmaX]        = boost::get<double>(toolOptions[cp_ToolProcFilterGaussianSigmaX]);
    fParams[cp_ToolProcFilterGaussianSigmaY]        = boost::get<double>(toolOptions[cp_ToolProcFilterGaussianSigmaY]);
    fParams[cp_ToolProcFilterBilateralSigmaColor]   = boost::get<double>(toolOptions[cp_ToolProcFilterBilateralSigmaColor]);
    fParams[cp_ToolProcFilterBilateralSigmaSpace]   = boost::get<double>(toolOptions[cp_ToolProcFilterBilateralSigmaSpace]);
}

void ThresholdTool::OnOptionChanged(const EvToolOption &e)
{
    const int toolId = boost::get<int>(e.toolOptions.at(cp_ToolId));
    if (kSpamID_TOOLBOX_PROC_THRESHOLD == toolId)
    {
        toolOptions = e.toolOptions;
        BoxToolImpl::ResetTool();
    }
}

void ThresholdTool::OnBoxingEnded(const EvBoxingEnded &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.mData.GetEventObject());
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (cav && frame)
    {
        const auto minGray = boost::get<int>(toolOptions[cp_ToolProcThresholdMin]);
        const auto maxGray = boost::get<int>(toolOptions[cp_ToolProcThresholdMax]);
        const auto channel = boost::get<int>(toolOptions[cp_ToolProcThresholdChannel]);
        cav->UpdateBinary(*(e.boxRect), minGray, maxGray, std::max(0, channel));
        frame->UpdateToolboxUI(kSpamID_TOOLBOX_PROC, kSpamID_TOOLBOX_PROC_THRESHOLD, cav->GetUUID(), e.boxRect);
        uuids.insert(cav->GetUUID());
    }
}

void ThresholdTool::OnImageClicked(const EvImageClicked &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (cav && frame)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        const auto minGray = boost::get<int>(toolOptions[cp_ToolProcThresholdMin]);
        const auto maxGray = boost::get<int>(toolOptions[cp_ToolProcThresholdMax]);
        const auto channel = boost::get<int>(toolOptions[cp_ToolProcThresholdChannel]);
        cav->UpdateBinary(Geom::Rect(imgPt.x, imgPt.y, imgPt.x+0.5, imgPt.y+0.5), minGray, maxGray, std::max(0, channel));
        frame->UpdateToolboxUI(kSpamID_TOOLBOX_PROC, kSpamID_TOOLBOX_PROC_THRESHOLD, cav->GetUUID(), Geom::OptRect());
        uuids.insert(cav->GetUUID());
    }
}

void ThresholdTool::OnEntityClicked(const EvEntityClicked &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.e.GetEventObject());
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (cav && frame)
    {
        const auto minGray = boost::get<int>(toolOptions[cp_ToolProcThresholdMin]);
        const auto maxGray = boost::get<int>(toolOptions[cp_ToolProcThresholdMax]);
        const auto channel = boost::get<int>(toolOptions[cp_ToolProcThresholdChannel]);
        Geom::PathVector pv;
        e.ent->BuildPath(pv);
        auto bbox = e.ent->GetBoundingBox();
        cav->UpdateBinary(bbox ? *bbox : Geom::Rect(), minGray, maxGray, std::max(0, channel));
        frame->UpdateToolboxUI(kSpamID_TOOLBOX_PROC, kSpamID_TOOLBOX_PROC_THRESHOLD, cav->GetUUID(), pv);
        uuids.insert(cav->GetUUID());
    }
}

sc::result ThresholdTool::react(const EvToolQuit &e)
{
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (frame)
    {
        for (const auto &uuid : uuids)
        {
            CairoCanvas *cav = frame->FindCanvasByUUID(uuid);
            if (cav)
            {
                cav->RemoveImageProcessData();
            }
        }
    }

    BoxToolT::QuitTool(e);
    return transit<NoTool>();
}
