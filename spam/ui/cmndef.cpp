#include "cmndef.h"
#include "spam.h"
#include <wx/stc/stc.h>

const std::string cp_ToolId                             = std::string("spam.tool.id");
const std::string cp_ToolRectMode                       = std::string("spam.tool.geom.rect.mode");
const std::string cp_ToolGeomFillPaint                  = std::string("spam.tool.geom.fill.paint");
const std::string cp_ToolGeomStrokePaint                = std::string("spam.tool.geom.stroke.paint");
const std::string cp_ToolGeomStrokeWidth                = std::string("spam.tool.geom.stroke.width");
const std::string cp_ToolGeomStrokeJoin                 = std::string("spam.tool.geom.stroke.join");
const std::string cp_ToolGeomStrokeCap                  = std::string("spam.tool.geom.stroke.cap");
const std::string cp_ToolGeomVertexEditMode             = std::string("spam.tool.geom.vertex.editmode");
const std::string cp_ToolProbeMode                      = std::string("spam.tool.probe.mode");
const std::string cp_ToolProcPyramidLevel               = std::string("spam.tool.process.pyramid.level");
const std::string cp_ToolProcThresholdMin               = std::string("spam.tool.process.threshold.min");
const std::string cp_ToolProcThresholdMax               = std::string("spam.tool.process.threshold.max");
const std::string cp_ToolProcThresholdChannel           = std::string("spam.tool.process.threshold.channel");
const std::string cp_ToolProcFilterType                 = std::string("spam.tool.process.filter.type");
const std::string cp_ToolProcFilterBorderType           = std::string("spam.tool.process.filter.border.type");
const std::string cp_ToolProcFilterBoxKernelWidth       = std::string("spam.tool.process.filter.box.kernel.width");
const std::string cp_ToolProcFilterBoxKernelHeight      = std::string("spam.tool.process.filter.box.kernel.height");
const std::string cp_ToolProcFilterGaussianKernelWidth  = std::string("spam.tool.process.filter.gaussian.kernel.width");
const std::string cp_ToolProcFilterGaussianKernelHeight = std::string("spam.tool.process.filter.gaussian.kernel.height");
const std::string cp_ToolProcFilterGaussianSigmaX       = std::string("spam.tool.process.filter.gaussian.sigma.x");
const std::string cp_ToolProcFilterGaussianSigmaY       = std::string("spam.tool.process.filter.gaussian.sigma.y");
const std::string cp_ToolProcFilterMedianKernelWidth    = std::string("spam.tool.process.filter.median.kernel.width");
const std::string cp_ToolProcFilterMedianKernelHeight   = std::string("spam.tool.process.filter.median.kernel.height");
const std::string cp_ToolProcFilterBilateralDiameter    = std::string("spam.tool.process.filter.bilateral.diameter");
const std::string cp_ToolProcFilterBilateralSigmaColor  = std::string("spam.tool.process.filter.bilateral.sigma.color");
const std::string cp_ToolProcFilterBilateralSigmaSpace  = std::string("spam.tool.process.filter.bilateral.sigma.space");
const std::string cp_ToolProcEdgeType                   = std::string("spam.tool.process.edge.type");
const std::string cp_ToolProcEdgeChannel                = std::string("spam.tool.process.edge.channel");
const std::string cp_ToolProcEdgeApertureSize           = std::string("spam.tool.process.edge.aperturesize");
const std::string cp_ToolProcEdgeCannyThresholdLow      = std::string("spam.tool.process.edge.canny.threshold.low");
const std::string cp_ToolProcEdgeCannyThresholdHigh     = std::string("spam.tool.process.edge.canny.threshold.high");
const std::string cp_ToolProcConvertChannel             = std::string("spam.tool.process.convert.channel");
const std::string cp_ToolProbeRegionMask                = std::string("spam.tool.probe.region.mask");
const std::string cp_Py3EditorStyle                     = std::string("spam.py3editor.style");
const std::string cp_Py3EditorRememberScriptPath        = std::string("spam.py3editor.scriptpath.remember");
const std::string cp_Py3EditorScriptFullPath            = std::string("spam.py3editor.scriptpath.fullpath");
const std::string cp_ThemeDarkMode                      = std::string("spam.theme.darkmode");

const std::string bm_Pointer                = std::string("tool.pointer");
const std::string bm_PointerEdit            = std::string("tool.node.editor");
const std::string bm_NodeEdit               = std::string("tool.node.editor");
const std::string bm_Box                    = std::string("box");
const std::string bm_Ellipse                = std::string("ellipse");
const std::string bm_Polygon                = std::string("polygon");
const std::string bm_Beziergon              = std::string("beziergon");
const std::string bm_Line                   = std::string("line");
const std::string bm_Arc                    = std::string("arc");
const std::string bm_Zigzagline             = std::string("zigzagline");
const std::string bm_Polyline               = std::string("polyline");
const std::string bm_Bezierline             = std::string("bezierline");
const std::string bm_NodeMove               = std::string("tool.node.editor");
const std::string bm_NodeAdd                = std::string("node.add");
const std::string bm_NodeDelete             = std::string("node.delete");
const std::string bm_NodeSmooth             = std::string("node.type.smooth");
const std::string bm_NodeCusp               = std::string("node.type.cusp");
const std::string bm_NodeSymmetric          = std::string("node.type.symmetric");
const std::string bm_ZoomIn                 = std::string("zoom.in");
const std::string bm_ZoomOut                = std::string("zoom.out");
const std::string bm_ZoomExtent             = std::string("zoom.fit.page");
const std::string bm_ZoomOriginal           = std::string("zoom.original");
const std::string bm_ZoomHalf               = std::string("zoom.half.size");
const std::string bm_ZoomDouble             = std::string("zoom.double.size");
const std::string bm_PathUnion              = std::string("path.union");
const std::string bm_PathDiff               = std::string("path.difference");
const std::string bm_PathInter              = std::string("path.intersection");
const std::string bm_PathXOR                = std::string("path.exclusion");
const std::string bm_PathCut                = std::string("path.cut");
const std::string bm_PathSlice              = std::string("path.division");
const std::string bm_ImageImport            = std::string("image.import");
const std::string bm_ImageExport            = std::string("image.export");

const std::string config_GetPy3EditorStyleFullName(const Py3EditorStyle styleType, const int shortNameId)
{
    const int darkMode = SpamConfig::Get<bool>(cp_ThemeDarkMode, true);
    const std::string darkModeStr = darkMode ? std::string(".dark") : std::string(".light");

    std::string styleName;
    switch (styleType)
    {
    case Py3EditorStyle::kPES_FONT:         styleName.assign(".font.fullname"); break;
    case Py3EditorStyle::kPES_BOLD:         styleName.assign(".font.bold");     break;
    case Py3EditorStyle::kPES_ITALIC:       styleName.assign(".font.italic");   break;
    case Py3EditorStyle::kPES_UNDERLINE:    styleName.assign(".font.underline");break;
    case Py3EditorStyle::kPES_BACKGROUND:   styleName.assign(".background");    break;
    case Py3EditorStyle::kPES_FOREGROUND:   styleName.assign(".foreground");    break;
    default: break;
    }

    std::string shortName;
    switch (shortNameId)
    {
    case wxSTC_STYLE_DEFAULT:       shortName.assign(".default");       break;
    case wxSTC_P_DEFAULT:           shortName.assign(".default");       break;
    case wxSTC_P_NUMBER:            shortName.assign(".number");        break;
    case wxSTC_P_STRING:            shortName.assign(".string");        break;
    case wxSTC_P_CHARACTER:         shortName.assign(".character");     break;
    case wxSTC_P_TRIPLE:            shortName.assign(".triple");        break;
    case wxSTC_P_TRIPLEDOUBLE:      shortName.assign(".tripledouble");  break;
    case wxSTC_P_CLASSNAME:         shortName.assign(".classname");     break;
    case wxSTC_P_DEFNAME:           shortName.assign(".defname");       break;
    case wxSTC_P_OPERATOR:          shortName.assign(".operator");      break;
    case wxSTC_P_IDENTIFIER:        shortName.assign(".identifier");    break;
    case wxSTC_P_COMMENTBLOCK:      shortName.assign(".comment.block"); break;
    case wxSTC_P_COMMENTLINE:       shortName.assign(".comment.line");  break;
    case wxSTC_P_WORD:              shortName.assign(".keyword");       break;
    case wxSTC_P_WORD2:             shortName.assign(".keyword2");      break;
    case wxSTC_P_DECORATOR:         shortName.assign(".decorator");     break;
    case wxSTC_P_STRINGEOL:         shortName.assign(".stringeol");     break;
    case wxSTC_STYLE_INDENTGUIDE:   shortName.assign(".indentguide");   break;
    case wxSTC_STYLE_LINENUMBER:    shortName.assign(".linenumber");    break;
    default:                        shortName.assign(".default");       break;
    }

    return cp_Py3EditorStyle + darkModeStr + shortName + styleName;
}

const wxColour config_GetPy3EditorForeground(const int shortNameId)
{
    wxColour defColor;
    const int darkMode = SpamConfig::Get<bool>(cp_ThemeDarkMode, true);
    if (darkMode)
    {
        switch (shortNameId)
        {
        case wxSTC_STYLE_DEFAULT:       defColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);    break;
        case wxSTC_P_DEFAULT:           defColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);    break;
        case wxSTC_P_NUMBER:            defColor = wxColour("RED");                                         break;
        case wxSTC_P_STRING:            defColor = wxColour(214, 157, 133);                                 break;
        case wxSTC_P_CHARACTER:         defColor = wxColour(214, 157, 133);                                 break;
        case wxSTC_P_TRIPLE:            defColor = wxColour(214, 157, 133);                                 break;
        case wxSTC_P_TRIPLEDOUBLE:      defColor = wxColour(214, 157, 133);                                 break;
        case wxSTC_P_CLASSNAME:         defColor = wxColour("TURQUOISE");                                   break;
        case wxSTC_P_DEFNAME:           defColor = wxColour(78, 201, 176);                                  break;
        case wxSTC_P_OPERATOR:          defColor = wxColour(128, 255, 255);                                 break;
        case wxSTC_P_IDENTIFIER:        defColor = wxColour("LIGHT GREY");                                  break;
        case wxSTC_P_COMMENTBLOCK:      defColor = wxColour("MEDIUM GREY");                                 break;
        case wxSTC_P_COMMENTLINE:       defColor = wxColour("MEDIUM GREY");                                 break;
        case wxSTC_P_WORD:              defColor = wxColour("YELLOW");                                      break;
        case wxSTC_P_WORD2:             defColor = wxColour("YELLOW");                                      break;
        case wxSTC_STYLE_INDENTGUIDE:   defColor = wxColour("GREY");                                        break;
        case wxSTC_STYLE_LINENUMBER:    defColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);    break;
        default:                        defColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);    break;
        }
    }
    else
    {
        defColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    }

    return SpamConfig::Get<wxColour>(config_GetPy3EditorStyleFullName(Py3EditorStyle::kPES_FOREGROUND, shortNameId), defColor);
}

const bool config_GetPy3EditorBold(const int shortNameId)
{
    bool useBold = false;
    switch (shortNameId)
    {
    case wxSTC_STYLE_DEFAULT:       useBold = false;    break;
    case wxSTC_P_DEFAULT:           useBold = false;    break;
    case wxSTC_P_NUMBER:            useBold = false;    break;
    case wxSTC_P_STRING:            useBold = false;    break;
    case wxSTC_P_CHARACTER:         useBold = false;    break;
    case wxSTC_P_TRIPLE:            useBold = false;    break;
    case wxSTC_P_TRIPLEDOUBLE:      useBold = false;    break;
    case wxSTC_P_CLASSNAME:         useBold = false;    break;
    case wxSTC_P_DEFNAME:           useBold = true;     break;
    case wxSTC_P_OPERATOR:          useBold = false;    break;
    case wxSTC_P_IDENTIFIER:        useBold = false;    break;
    case wxSTC_P_COMMENTBLOCK:      useBold = false;    break;
    case wxSTC_P_COMMENTLINE:       useBold = false;    break;
    case wxSTC_P_WORD:              useBold = true;     break;
    case wxSTC_P_WORD2:             useBold = true;     break;
    case wxSTC_STYLE_INDENTGUIDE:   useBold = false;    break;
    case wxSTC_STYLE_LINENUMBER:    useBold = false;    break;
    default:                        useBold = false;    break;
    }

    return SpamConfig::Get<bool>(config_GetPy3EditorStyleFullName(Py3EditorStyle::kPES_BOLD, shortNameId), useBold);
}

const wxColour config_GetPy3EditorBackground(const int shortNameId)
{
    wxColour defColor = SpamConfig::Get<wxColour>(config_GetPy3EditorStyleFullName(Py3EditorStyle::kPES_BACKGROUND, wxSTC_P_DEFAULT), wxSystemSettings::GetColour(wxSYS_COLOUR_DESKTOP));
    return SpamConfig::Get<wxColour>(config_GetPy3EditorStyleFullName(Py3EditorStyle::kPES_BACKGROUND, shortNameId), defColor);
}

const wxFont config_GetPy3EditorFont(const int shortNameId)
{
    wxFont defFont = SpamConfig::Get<wxFont>(config_GetPy3EditorStyleFullName(Py3EditorStyle::kPES_FONT, wxSTC_P_DEFAULT), *wxSWISS_FONT);
    return SpamConfig::Get<wxFont>(config_GetPy3EditorStyleFullName(Py3EditorStyle::kPES_FONT, shortNameId), defFont);
}
