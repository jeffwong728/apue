#ifndef SPAM_UI_COMMON_DEFINE_H
#define SPAM_UI_COMMON_DEFINE_H
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

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
    spamID_VIEW_TOOLBOX_BAR,
    spamID_VIEW_DEFAULT_LAYOUT,
    spamID_VIEW_SET_TILE_LAYOUT,
    spamID_VIEW_TILE_LAYOUT,
    spamID_VIEW_STACK_LAYOUT,

    kSpamID_SCALE_CHOICE,
    kSpamID_ZOOM_IN,
    kSpamID_ZOOM_OUT,
    kSpamID_ZOOM_EXTENT,
    kSpamID_ZOOM_ORIGINAL,
    kSpamID_ZOOM_HALF,
    kSpamID_ZOOM_DOUBLE,

    kSpamID_MAIN_SCALE_CHOICE,
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
    
    kSpamID_STATUS_CHECKBOX,
    
    kSpamID_ADD_STATION,
    kSpamID_DELETE_ENTITIES,
    kSpamID_SHOW_ENTITIES,
    kSpamID_HIDE_ENTITIES,
    kSpamID_SHOW_ONLY_ENTITIES,
    kSpamID_SHOW_REVERSE_ENTITIES,
    kSpamID_SHOW_ALL_ENTITIES,
    kSpamID_HIDE_ALL_ENTITIES,

    kSpamID_TOOLPAGE_PROBE,
    kSpamID_TOOLPAGE_GEOM,
    kSpamID_TOOLPAGE_MATCH,
    kSpamID_TOOLPAGE_MEASURE,
    kSpamID_TOOLPAGE_STYLE,
    kSpamID_TOOLPAGE_GUARD,

    kSpamID_TOOLBOX_PROBE,
    kSpamID_TOOLBOX_GEOM,
    kSpamID_TOOLBOX_MATCH,
    kSpamID_TOOLBOX_MEASURE,
    kSpamID_TOOLBOX_STYLE,

    kSpamID_TOOLBOX_PROBE_SELECT,
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
    kSpamID_TOOLBOX_GEOM_GUARD,

    kSpamID_TOOLBOX_GEOM_FILL_COLOR,
    kSpamID_TOOLBOX_GEOM_FILL_ALPHA,
    kSpamID_TOOLBOX_GEOM_STROKE_WIDTH,
    kSpamID_TOOLBOX_GEOM_STROKE_COLOR,
    kSpamID_TOOLBOX_GEOM_STROKE_ALPHA,

    kSpamID_TOOLBOX_RECT_MODE,

    kSpamID_TOOLBOX_MATCH_GRAY,
    kSpamID_TOOLBOX_MATCH_SHAPE,
    kSpamID_TOOLBOX_MATCH_GUARD
};

enum
{
    kSpamENTITY_TYPE_IMAGE,

    kSpamENTITY_TYPE_GUARD
};

enum
{
    kSpamTOOL_RECT_MODE_2POINTS,
    kSpamTOOL_RECT_MODE_3POINTS
};

extern const std::string cp_ToolRectMode;
extern const std::string cp_ToolGeomFillPaint;
extern const std::string cp_ToolGeomStrokePaint;
extern const std::string cp_ToolGeomStrokeWidth;
extern const std::string cp_ToolGeomStrokeJoin;
extern const std::string cp_ToolGeomStrokeCap;

#endif //SPAM_UI_COMMON_DEFINE_H