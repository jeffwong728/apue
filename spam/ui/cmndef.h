#ifndef SPAM_UI_COMMON_DEFINE_H
#define SPAM_UI_COMMON_DEFINE_H
#include "errdef.h"
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <boost/mpl/vector.hpp>
#include <boost/variant.hpp>
#include <string>
#include <unordered_map>

enum
{
    ID_Hello = 1,

    kSpamGlobalMainToolBar,
    kSpamGlobalImageToolBar,
    kSpamMainToolBarNoteBook,
    kSpamMainToolBarProcessPage,
    kSpamMainToolBarMatchPage,
    kSpamMainToolBarMeasurePage,

    kSpamImageToolBar,
    kSpamImageCanvas,

    kSpamOpenProjDBFileLabelCtrl,
    kSpamOpenProjDBFilePickerCtrl,
    kSpamOpenProjProjLabelCtrl,
    kSpamOpenProjProjListBoxCtrl,
    kSpamOpenProjStdBtnBox,

    kSpamLogToolBar,
    kSpamLogTextCtrl,
    kSpamConsoleTextCtrl,
    kSpamConsoleTimeCtrl,

    kSpamPyEditorCtrl,
    kSpamPreferencesTreeBookCtrl,

    kPy3EditorStyleCategoryCtrl,
    kPy3EditorStyleFontCtrl,
    kPy3EditorStyleSampleCtrl,
    kPy3EditorStyleBoldCtrl,
    kPy3EditorStyleItalicCtrl,
    kPy3EditorStyleUnderlineCtrl,
    kPy3EditorStyleForegroundColorCtrl,
    kPy3EditorStyleBackgroundColorCtrl,

    kStationThumbnailToolBar,
    kStationThumbnailCtrl,
    
    kSpamProjToolBar,
    kSpamProjTree,
    kSpamToolboxBar
};

enum
{
    spamID_LOAD_IMAGE = wxID_HIGHEST + 1,
    spamID_EXPORT_IMAGE,
    spamID_BASE_IMAGE_FILE_HISTORY,
    spamID_VIEW_MAIN_TOOL = spamID_BASE_IMAGE_FILE_HISTORY+100,
    spamID_VIEW_IMAGE,
    spamID_VIEW_PROJECT,
    spamID_VIEW_LOG,
    spamID_VIEW_CONSOLE,
    spamID_VIEW_PYEDITOR,
    spamID_VIEW_IMAGES_ZONE,
    spamID_VIEW_TOOLBOX_BAR,
    spamID_VIEW_DEFAULT_LAYOUT,
    spamID_VIEW_SET_TILE_LAYOUT,
    spamID_VIEW_TILE_LAYOUT,
    spamID_VIEW_STACK_LAYOUT,

    kSpamID_SCALE_CHOICE,
    kSpamID_ZOOM,
    kSpamID_ZOOM_IN,
    kSpamID_ZOOM_OUT,
    kSpamID_ZOOM_EXTENT,
    kSpamID_ZOOM_ORIGINAL,
    kSpamID_ZOOM_HALF,
    kSpamID_ZOOM_DOUBLE,

    kSpamID_MAIN_SCALE_CHOICE,
    kSpamID_MAIN_ZOOM,
    kSpamID_MAIN_ZOOM_IN,
    kSpamID_MAIN_ZOOM_OUT,
    kSpamID_MAIN_ZOOM_EXTENT,
    kSpamID_MAIN_ZOOM_ORIGINAL,
    kSpamID_MAIN_ZOOM_HALF,
    kSpamID_MAIN_ZOOM_DOUBLE,
    
    kSpamID_FILE_PICKER,
    kSpamID_PROJ_LIST_BOX,
    
    kSpamID_LOG_CLEAR,
    kSpamID_LOG_SAVE,
    
    kSpamID_STATUS_STATUS,
    kSpamID_STATUS_CHECKBOX,
    
    kSpamID_ADD_STATION,
    kSpamID_DELETE_ENTITIES,
    kSpamID_SHOW_ENTITIES,
    kSpamID_HIDE_ENTITIES,
    kSpamID_SHOW_ONLY_ENTITIES,
    kSpamID_SHOW_REVERSE_ENTITIES,
    kSpamID_SHOW_ALL_ENTITIES,
    kSpamID_HIDE_ALL_ENTITIES,
    kSpamID_PUSH_TO_BACK,
    kSpamID_BRING_TO_FRONT,

    kSpamID_TOOLPAGE_PROBE,
    kSpamID_TOOLPAGE_GEOM,
    kSpamID_TOOLPAGE_PROC,
    kSpamID_TOOLPAGE_MATCH,
    kSpamID_TOOLPAGE_MEASURE,
    kSpamID_TOOLPAGE_STYLE,
    kSpamID_TOOLPAGE_GUARD,

    kSpamID_TOOLBOX_PROBE,
    kSpamID_TOOLBOX_GEOM,
    kSpamID_TOOLBOX_PROC,
    kSpamID_TOOLBOX_MATCH,
    kSpamID_TOOLBOX_MEASURE,
    kSpamID_TOOLBOX_STYLE,

    kSpamID_TOOLBOX_PROBE_SELECT,
    kSpamID_TOOLBOX_PROBE_REGION,
    kSpamID_TOOLBOX_PROBE_HISTOGRAM,
    kSpamID_TOOLBOX_PROBE_GUARD,

    kSpamID_TOOLBOX_GEOM_TRANSFORM,
    kSpamID_TOOLBOX_GEOM_EDIT,
    kSpamID_TOOLBOX_GEOM_RECT,
    kSpamID_TOOLBOX_GEOM_ELLIPSE,
    kSpamID_TOOLBOX_GEOM_POLYGON,
    kSpamID_TOOLBOX_GEOM_BEZIERGON,
    kSpamID_TOOLBOX_GEOM_LINE,
    kSpamID_TOOLBOX_GEOM_ARC,
    kSpamID_TOOLBOX_GEOM_ZIGZAGLINE,
    kSpamID_TOOLBOX_GEOM_POLYLINE,
    kSpamID_TOOLBOX_GEOM_BEZIERLINE,
    kSpamID_TOOLBOX_GEOM_UNION,
    kSpamID_TOOLBOX_GEOM_INTERS,
    kSpamID_TOOLBOX_GEOM_DIFF,
    kSpamID_TOOLBOX_GEOM_SYMDIFF,
    kSpamID_TOOLBOX_GEOM_GUARD,

    kSpamID_TOOLBOX_PROC_ENHANCEMENT,
    kSpamID_TOOLBOX_PROC_THRESHOLD,
    kSpamID_TOOLBOX_PROC_FILTER,
    kSpamID_TOOLBOX_PROC_EDGE,
    kSpamID_TOOLBOX_PROC_PYRAMID,
    kSpamID_TOOLBOX_PROC_GUARD,

    kSpamID_TOOLBOX_NODE_MOVE,
    kSpamID_TOOLBOX_NODE_ADD,
    kSpamID_TOOLBOX_NODE_DELETE,
    kSpamID_TOOLBOX_NODE_SMOOTH,
    kSpamID_TOOLBOX_NODE_CUSP,
    kSpamID_TOOLBOX_NODE_SYMMETRIC,

    kSpamID_TOOLBOX_PROBE_PIXEL,
    kSpamID_TOOLBOX_PROBE_ENTITY,
    kSpamID_TOOLBOX_PROBE_IMAGE,

    kSpamID_TOOLBOX_PROBE_REGION_AREA,
    kSpamID_TOOLBOX_PROBE_REGION_CIRCULARITY,
    kSpamID_TOOLBOX_PROBE_REGION_CONVEXITY,
    kSpamID_TOOLBOX_PROBE_REGION_BBOX,
    kSpamID_TOOLBOX_PROBE_REGION_CENTROID,
    kSpamID_TOOLBOX_PROBE_REGION_CONVEX,
    kSpamID_TOOLBOX_PROBE_REGION_DIAMETER,
    kSpamID_TOOLBOX_PROBE_REGION_SMALLESTRECT,
    kSpamID_TOOLBOX_PROBE_REGION_SMALLESTCIRCLE,
    kSpamID_TOOLBOX_PROBE_REGION_ORIENTATION,
    kSpamID_TOOLBOX_PROBE_REGION_ELLIPTIC_AXIS,

    kSpamID_TOOLBOX_GEOM_FILL_COLOR,
    kSpamID_TOOLBOX_GEOM_FILL_ALPHA,
    kSpamID_TOOLBOX_GEOM_STROKE_WIDTH,
    kSpamID_TOOLBOX_GEOM_STROKE_COLOR,
    kSpamID_TOOLBOX_GEOM_STROKE_ALPHA,

    kSpamID_TOOLBOX_RECT_MODE,

    kSpamID_TOOLBOX_MATCH_GRAY,
    kSpamID_TOOLBOX_MATCH_SHAPE,
    kSpamID_TOOLBOX_MATCH_GUARD,

    kSpamID_STATION_THUMBNAIL_DELETE,
    kSpamID_STATION_THUMBNAIL_SAVE,

    kSpamID_PY3_SCRIPT_PLAY
};

enum SpamIconPurpose
{
    kICON_PURPOSE_TOOLBOX,
    kICON_PURPOSE_TOOLBAR,
    kICON_PURPOSE_CURSOR,

    kICON_PURPOSE_GUARD
};

enum class SpamEntityType
{
    kET_IMAGE,
    kET_GEOM,
    kET_GEOM_RECT,
    kET_GEOM_ELLIPSE,
    kET_GEOM_POLYGON,
    kET_GEOM_BEZIERGON,
    kET_GEOM_LINE,
    kET_GEOM_ARC,
    kET_GEOM_ZIGZAGLINE,
    kET_GEOM_POLYLINE,
    kET_GEOM_BEZIERLINE,
    kET_REGION,
    kET_CONTOUR,

    kET_GUARD
};

enum class SpamEntitySelectionMode
{
    kESM_NONE,
    kESM_MULTIPLE,
    kESM_SINGLE,
    kESM_BOX_SINGLE,

    kESM_GUARD
};

enum class SpamEntityOperation
{
    kEO_GENERAL,
    kEO_GEOM_CREATE,
    kEO_GEOM_TRANSFORM,
    kEO_VERTEX_MOVE,
    kEO_VERTEX_ADD,
    kEO_VERTEX_DELETE,
    kEO_VERTEX_SMOOTH,
    kEO_VERTEX_CUSP,
    kEO_VERTEX_SYMMETRIC,
    kEO_REGION_PROBE,
    kEO_CONTOUR_PROBE,

    kEO_GUARD
};

enum class SpamRectToolMode
{
    kRTM_2POINTS,
    kRTM_3POINTS
};

enum class StatusIconType
{
    kSIT_NONE,
    kSIT_OK,
    kSIT_ERROR,
    kSIT_WARNING,
    kSIT_INFO
};

enum class Py3EditorStyle
{
    kPES_FONT,
    kPES_BOLD,
    kPES_ITALIC,
    kPES_UNDERLINE,
    kPES_BACKGROUND,
    kPES_FOREGROUND
};

enum class RegionFeatureFlag : uint64_t
{
    kRFF_AREA = 0x1,
    kRFF_LENGTH = 0x2,
    kRFF_RECT1 = 0x4,
    kRFF_RECT2 = 0x8,
    kRFF_SMALLEST_CIRCLE = 0x10,
    kRFF_ORIENTATION = 0x20,
    kRFF_ELLIPTIC_AXIS = 0x40,
    kRFF_CONVEX_HULL = 0x80,
    kRFF_DIAMETER = 0x100,
    kRFF_CENTROID = 0x200,
    kRFF_CIRCULARITY = 0x400,
    kRFF_CONVEXITY = 0x800,
    kRFF_ALL_FEATURES = 0xFFFFFFFFFFFFFFFF
};

typedef boost::mpl::vector<int, long, double, uint64_t> OptTypes0;
typedef boost::mpl::push_front<OptTypes0, std::string>::type OptTypes;
using ToolOptions = std::unordered_map<std::string, boost::make_variant_over<OptTypes>::type>;

extern const std::string cp_ToolId;
extern const std::string cp_ToolRectMode;
extern const std::string cp_ToolGeomFillPaint;
extern const std::string cp_ToolGeomStrokePaint;
extern const std::string cp_ToolGeomStrokeWidth;
extern const std::string cp_ToolGeomStrokeJoin;
extern const std::string cp_ToolGeomStrokeCap;
extern const std::string cp_ToolGeomVertexEditMode;
extern const std::string cp_ToolProbeMode;
extern const std::string cp_ToolProbeRegionMask;
extern const std::string cp_ThemeDarkMode;
extern const std::string cp_Py3EditorStyle;
extern const std::string cp_Py3EditorRememberScriptPath;
extern const std::string cp_Py3EditorScriptFullPath;

extern const std::string bm_Pointer;
extern const std::string bm_PointerEdit;
extern const std::string bm_NodeEdit;
extern const std::string bm_Box;
extern const std::string bm_Ellipse;
extern const std::string bm_Polygon;
extern const std::string bm_Beziergon;
extern const std::string bm_Line;
extern const std::string bm_Arc;
extern const std::string bm_Zigzagline;
extern const std::string bm_Polyline;
extern const std::string bm_Bezierline;
extern const std::string bm_NodeMove;
extern const std::string bm_NodeAdd;
extern const std::string bm_NodeDelete;
extern const std::string bm_NodeSmooth;
extern const std::string bm_NodeCusp;
extern const std::string bm_NodeSymmetric;
extern const std::string bm_ZoomIn;
extern const std::string bm_ZoomOut;
extern const std::string bm_ZoomExtent;
extern const std::string bm_ZoomOriginal;
extern const std::string bm_ZoomHalf;
extern const std::string bm_ZoomDouble;
extern const std::string bm_PathUnion;
extern const std::string bm_PathDiff;
extern const std::string bm_PathInter;
extern const std::string bm_PathXOR;
extern const std::string bm_PathCut;
extern const std::string bm_PathSlice;
extern const std::string bm_ImageImport;
extern const std::string bm_ImageExport;

extern const std::string config_GetPy3EditorStyleFullName(const Py3EditorStyle styleType, const int shortNameId);
extern const bool config_GetPy3EditorBold(const int shortNameId);
extern const wxColour config_GetPy3EditorForeground(const int shortNameId);
extern const wxColour config_GetPy3EditorBackground(const int shortNameId);
extern const wxFont config_GetPy3EditorFont(const int shortNameId);

#endif //SPAM_UI_COMMON_DEFINE_H