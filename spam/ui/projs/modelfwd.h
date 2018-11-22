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
#include <boost/any.hpp>

class NodeFactory;
class ModelNode;
class ProjNode;
class StationNode;
class DrawableNode;
class RectNode;
class GenericEllipseArcNode;
class PolygonNode;
class GeomNode;
class wxDataViewModel;
class ProjTreeModel;

typedef std::shared_ptr<ModelNode>             SPModelNode;
typedef std::shared_ptr<const ModelNode>       SPCModelNode;
typedef std::shared_ptr<ProjNode>              SPProjNode;
typedef std::shared_ptr<StationNode>           SPStationNode;
typedef std::shared_ptr<DrawableNode>          SPDrawableNode;
typedef std::shared_ptr<RectNode>              SPRectNode;
typedef std::shared_ptr<GenericEllipseArcNode> SPGenericEllipseArcNode;
typedef std::shared_ptr<PolygonNode>           SPPolygonNode;
typedef std::shared_ptr<GeomNode>              SPGeomNode;
typedef std::weak_ptr<ModelNode>               WPModelNode;
typedef std::weak_ptr<ProjNode>                WPProjNode;
typedef std::weak_ptr<StationNode>             WPStationNode;
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

struct RectData
{
    std::array<std::array<double, 2>, 4> points;
    std::array<std::array<double, 2>, 4> radii;
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

using NodeIdVector = std::vector<NodeId>;

#endif //SPAM_UI_PROJS_MODEL_FWD_H