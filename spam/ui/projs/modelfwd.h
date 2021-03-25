#ifndef SPAM_UI_PROJS_MODEL_FWD_H
#define SPAM_UI_PROJS_MODEL_FWD_H
#include <memory>
#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <array>
#include <vector>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/mvlab.hpp>
#include <boost/any.hpp>
#include <ui/cmndef.h>
namespace Geom {
    class PathVector;
    class Rect;
    class Curve;
    class OptRect;
}

class NodeFactory;
class ModelNode;
class ProjNode;
class StationNode;
class DrawableNode;
class RectNode;
class LineNode;
class GenericEllipseArcNode;
class PolygonNode;
class BeziergonNode;
class GeomNode;
class wxDataViewModel;
class ProjTreeModel;

typedef std::shared_ptr<ModelNode>             SPModelNode;
typedef std::shared_ptr<const ModelNode>       SPCModelNode;
typedef std::shared_ptr<ProjNode>              SPProjNode;
typedef std::shared_ptr<StationNode>           SPStationNode;
typedef std::shared_ptr<DrawableNode>          SPDrawableNode;
typedef std::shared_ptr<RectNode>              SPRectNode;
typedef std::shared_ptr<LineNode>              SPLineNode;
typedef std::shared_ptr<GenericEllipseArcNode> SPGenericEllipseArcNode;
typedef std::shared_ptr<PolygonNode>           SPPolygonNode;
typedef std::shared_ptr<BeziergonNode>         SPBeziergonNode;
typedef std::shared_ptr<GeomNode>              SPGeomNode;
typedef std::weak_ptr<ModelNode>               WPModelNode;
typedef std::weak_ptr<ProjNode>                WPProjNode;
typedef std::weak_ptr<StationNode>             WPStationNode;
typedef std::weak_ptr<GeomNode>                WPGeomNode;
typedef std::weak_ptr<RectNode>                WPRectNode;
typedef std::weak_ptr<GenericEllipseArcNode>   WPGenericEllipseArcNode;
typedef std::weak_ptr<DrawableNode>            WPDrawableNode;
typedef std::vector<SPModelNode>               SPModelNodeVector;
typedef std::vector<SPProjNode>                SPProjNodeVector;
typedef std::vector<SPStationNode>             SPStationNodeVector;
typedef std::vector<SPGeomNode>                SPGeomNodeVector;
typedef std::vector<SPDrawableNode>            SPDrawableNodeVector;
typedef std::vector<WPModelNode>               WPModelNodeVector;
typedef std::vector<WPProjNode>                WPProjNodeVector;
typedef std::vector<WPStationNode>             WPStationNodeVector;
typedef std::vector<boost::any>                SpamMany;

enum class SelectionState
{
    kSelNone,
    kSelState,
    kSelScale,
    kSelRotateAndSkew,
    kSelNodeEdit
};

enum class HitState
{
    kHsNone,
    kHsNode,
    kHsEdge,
    kHsFace,
    kHsScaleHandle,
    kHsRotateHandle,
    kHsSkewHandle
};

enum class HighlightState
{
    kHlNone,
    kHlNode,
    kHlEdge,
    kHlFace,
    kHlScaleHandle,
    kHlRotateHandle,
    kHlSkewHandle
};

enum class EntitySigType
{
    kEntityCreate,
    kEntityAdd,
    kEntityDelete,
    kStationAdd,
    kStationDelete,
    kGeomCreate,
    kGeomAdd,
    kGeomDelete,

    kGuard
};

enum class GenericEllipseArcType {
    kAtSlice,
    kAtChord,
    kAtArc
};

enum class BezierNodeType {
    kBezierNoneCtrl = 0b00,
    kBezierPrevCtrl = 0b10,
    kBezierNextCtrl = 0b01,
    kBezierBothCtrl = 0b11
};

struct RectData
{
    std::array<std::array<double, 2>, 4> points;
    std::array<std::array<double, 2>, 4> radii;
    std::array<double, 6> transform;
};

struct LineData
{
    std::array<std::array<double, 2>, 2> points;
    std::array<double, 6> transform;
};

struct PolygonData
{
    std::vector<std::array<double, 2>> points;
    std::array<double, 6> transform;
};

struct GenericEllipseArcData
{
    std::array<std::array<double, 2>, 4> points;
    std::array<double, 2> angles;
    std::array<double, 6> transform;
    GenericEllipseArcType type;
};

struct BezierData
{
    std::vector<int> ntypes;
    std::vector<std::array<double, 6>> points;
    std::vector<int> cvertices;
    std::array<double, 6> transform;
    GenericEllipseArcType type;
};

struct HighlightData
{
    HighlightState hls;
    int id;
    int subid;
};

struct SelectionData
{
    SelectionState ss;
    HitState       hs;
    int id;
    int subid;
    int master;
};

struct NodeId
{
    int id;
    int subid;
};

struct ImageBufferItem
{
    SpamEntityType iType;
    wxString iName;
    wxString iStationUUID;
    cv::Mat  iSrcImage;
    wxBitmap iThumbnail;
};

using NodeIdVector    = std::vector<NodeId>;
using CurveVector     = std::vector<std::unique_ptr<Geom::Curve>>;
using ImageBufferZone = std::unordered_map<std::string, ImageBufferItem>;
using RgnBufferZone = std::unordered_map<std::string, cv::Ptr<cv::mvlab::Region>>;

#endif //SPAM_UI_PROJS_MODEL_FWD_H