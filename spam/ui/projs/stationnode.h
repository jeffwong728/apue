#ifndef SPAM_UI_PROJS_STATION_NODE_H
#define SPAM_UI_PROJS_STATION_NODE_H
#include "modelfwd.h"
#include "modelnode.h"
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/2geom.h>
#pragma warning( pop )

class StationNode : public ModelNode
{
public:
    StationNode() : ModelNode() {}
    StationNode(const SPModelNode &parent) : ModelNode(parent) {}
    StationNode(const SPModelNode &parent, const wxString &title);
    ~StationNode();

public:
    bool IsContainer() const override { return true; }
    bool IsCurrentStation() const override { return current_; }
    EntitySigType GetAddSigType() const wxOVERRIDE;
    EntitySigType GetDeleteSigType() const wxOVERRIDE;
    SPDrawableNodeVector GeSelected() const;
    int GetNumSelected() const;
    int GetNumDrawable() const;

    void SetTaberName(const std::string &taberName) { tabContainerName_ = taberName; }
    const std::string &GetTaberName() const { return tabContainerName_; }
    std::string &GetTaberName() { return tabContainerName_; }

    void SetImage(const cv::Mat &img) { img_ = img; }
    const cv::Mat &GetImage() const { return img_; }
    cv::Mat &GetImage() { return img_; }
    SPDrawableNode FindDrawable(const std::string &name);
    SPDrawableNode FindDrawable(const Geom::Point &pt);
    SPDrawableNode FindDrawable(const Geom::Point &pt, const double sx, const double sy, SelectionData &sd);
    void SelectDrawable(const Geom::Rect &box, SPDrawableNodeVector &ents);
    void SetColor(const wxColour &color) { drawStyle_.strokeColor_ = color; }
    wxColour GetColor() const { return drawStyle_.strokeColor_; }
    wxColour GetFillColor() const { return drawStyle_.fillColor_; }
    void SetColor(const std::vector<wxColour> &colors) { multiColors_ = colors; }
    void SetColored(const int number_of_colors = 68);
    double GetLineWidth() const { return drawStyle_.strokeWidth_; }
    void SetLineWidth(const double width) { drawStyle_.strokeWidth_ = width; }
    wxColour GetNextColor() const;
    void SetDraw(const std::string &drawMode) { drawMode_ = drawMode; }
    std::string GetDraw() const { return drawMode_; }

public:
    void Save(const H5::Group &g) const override;
    void Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me) override;

    static std::string GetTypeName() { return std::string("StationNode"); }
    static SPModelNode Create(const SPModelNode &parent, const wxString &title)
    { 
        return std::make_shared<StationNode>(parent, title);
    }

private:
    void SaveImage(const H5::Group &cwg) const;
    void ImportImage(const H5::Group &cwg);
    void SaveGrayScaleImage(const H5::Group &cwg) const;
    void SaveRGBImage(const H5::Group &cwg) const;
    void SaveRGBAImage(const H5::Group &cwg) const;

public:
    bool current_;
    mutable int currentColorIndex_ = 0;
    std::string drawMode_;
    std::vector<wxColour> multiColors_;
    std::string tabContainerName_;
    cv::Mat img_;
};

#endif //SPAM_UI_PROJS_STATION_NODE_H