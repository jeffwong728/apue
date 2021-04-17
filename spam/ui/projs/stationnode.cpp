#include "stationnode.h"
#include "drawablenode.h"
#include <ui/evts.h>
#include <ui/spam.h>
#include <helper/h5db.h>
#include <helper/commondef.h>

StationNode::StationNode(const SPModelNode &parent, const wxString &title)
    : ModelNode(parent, title)
    , current_(false)
    , drawMode_("fill")
{
    SetColored();
}

StationNode::~StationNode()
{
}

EntitySigType StationNode::GetAddSigType() const
{
    return EntitySigType::kStationAdd;
}

EntitySigType StationNode::GetDeleteSigType() const
{
    return EntitySigType::kStationDelete;
}

SPDrawableNodeVector StationNode::GeSelected() const
{
    SPDrawableNodeVector sels;
    for (auto &c : GetChildren())
    {
        auto drawable = std::dynamic_pointer_cast<DrawableNode>(c);
        if (drawable && drawable->IsSelected())
        {
            sels.push_back(drawable);
        }
    }

    return sels;
}

int StationNode::GetNumSelected() const
{
    int numSel = 0;
    for (auto &c : GetChildren())
    {
        auto drawable = std::dynamic_pointer_cast<DrawableNode>(c);
        if (drawable && drawable->IsSelected())
        {
            numSel += 1;
        }
    }

    return numSel;
}

int StationNode::GetNumDrawable() const
{
    int numDra = 0;
    for (auto &c : GetChildren())
    {
        auto drawable = std::dynamic_pointer_cast<DrawableNode>(c);
        if (drawable)
        {
            numDra += 1;
        }
    }

    return numDra;
}

SPDrawableNode StationNode::FindDrawable(const std::string &name)
{
    for (auto &c : GetChildren())
    {
        auto drawable = std::dynamic_pointer_cast<DrawableNode>(c);
        if (drawable && drawable->GetTitle() == name)
        {
            return drawable;
        }
    }

    return SPDrawableNode();
}

SPDrawableNode StationNode::FindDrawable(const Geom::Point &pt)
{
    const SelectionFilter *sf = Spam::GetSelectionFilter();
    for (auto &c : GetChildren())
    {
        auto drawable = std::dynamic_pointer_cast<DrawableNode>(c);
        if (sf && sf->IsPass(drawable))
        {
            auto hl = drawable->HitTest(pt);
            if (hl.hs != HitState::kHsNone)
            {
                return drawable;
            }
        }
    }

    return SPDrawableNode();
}

SPDrawableNode StationNode::FindDrawable(const Geom::Point &pt, const double sx, const double sy, SelectionData &sd)
{
    sd.ss = SelectionState::kSelNone;
    sd.hs = HitState::kHsNone;
    sd.id = -1;
    sd.subid = -1;
    sd.master = 0;

    const SelectionFilter *sf = Spam::GetSelectionFilter();
    for (auto &c : GetChildren())
    {
        auto drawable = std::dynamic_pointer_cast<DrawableNode>(c);
        if (sf && sf->IsPass(drawable))
        {
            auto ht = drawable->HitTest(pt, sx, sy);
            if (ht.hs != HitState::kHsNone)
            {
                sd = ht;
                return drawable;
            }
        }
    }

    return SPDrawableNode();
}

void StationNode::SelectDrawable(const Geom::Rect &box, SPDrawableNodeVector &ents)
{
    ents.clear();
    const SelectionFilter *sf = Spam::GetSelectionFilter();
    for (auto &c : GetChildren())
    {
        auto drawable = std::dynamic_pointer_cast<DrawableNode>(c);
        if (sf && sf->IsPass(drawable))
        {
            if (drawable->IsIntersection(box))
            {
                ents.push_back(drawable);
            }
        }
    }
}

void StationNode::SetColored(const int number_of_colors)
{
    std::vector<wxColour> multiColors;
    multiColors.reserve(64);
    multiColors.emplace_back(wxColour("RED"));
    multiColors.emplace_back(wxColour("AQUAMARINE"));
    multiColors.emplace_back(wxColour("BLUE"));
    multiColors.emplace_back(wxColour("BLUE VIOLET"));
    multiColors.emplace_back(wxColour("BROWN"));
    multiColors.emplace_back(wxColour("CADET BLUE"));
    multiColors.emplace_back(wxColour("YELLOW"));
    multiColors.emplace_back(wxColour("CORAL"));
    multiColors.emplace_back(wxColour("CORNFLOWER BLUE"));
    multiColors.emplace_back(wxColour("CYAN"));
    multiColors.emplace_back(wxColour("ORANGE"));
    multiColors.emplace_back(wxColour("PINK"));
    multiColors.emplace_back(wxColour("DARK GREEN"));
    multiColors.emplace_back(wxColour("DARK OLIVE GREEN"));
    multiColors.emplace_back(wxColour("DARK ORCHID"));
    multiColors.emplace_back(wxColour("DARK SLATE BLUE"));
    multiColors.emplace_back(wxColour("DARK TURQUOISE"));
    multiColors.emplace_back(wxColour("FIREBRICK"));
    multiColors.emplace_back(wxColour("FOREST GREEN"));
    multiColors.emplace_back(wxColour("GREEN"));
    multiColors.emplace_back(wxColour("GREEN YELLOW"));
    multiColors.emplace_back(wxColour("INDIAN RED"));
    multiColors.emplace_back(wxColour("KHAKI"));
    multiColors.emplace_back(wxColour("LIGHT BLUE"));
    multiColors.emplace_back(wxColour("LIGHT STEEL BLUE"));
    multiColors.emplace_back(wxColour("LIME GREEN"));
    multiColors.emplace_back(wxColour("MAGENTA"));
    multiColors.emplace_back(wxColour("MAROON"));
    multiColors.emplace_back(wxColour("MEDIUM AQUAMARINE"));
    multiColors.emplace_back(wxColour("MEDIUM BLUE"));
    multiColors.emplace_back(wxColour("MEDIUM FOREST GREEN"));
    multiColors.emplace_back(wxColour("MEDIUM GOLDENROD"));
    multiColors.emplace_back(wxColour("MEDIUM ORCHID"));
    multiColors.emplace_back(wxColour("MEDIUM SEA GREEN"));
    multiColors.emplace_back(wxColour("MEDIUM SLATE BLUE"));
    multiColors.emplace_back(wxColour("MEDIUM SPRING GREEN"));
    multiColors.emplace_back(wxColour("MEDIUM TURQUOISE"));
    multiColors.emplace_back(wxColour("MEDIUM VIOLET RED"));
    multiColors.emplace_back(wxColour("MIDNIGHT BLUE"));
    multiColors.emplace_back(wxColour("NAVY"));
    multiColors.emplace_back(wxColour("ORANGE RED"));
    multiColors.emplace_back(wxColour("ORCHID"));
    multiColors.emplace_back(wxColour("PALE GREEN"));
    multiColors.emplace_back(wxColour("PLUM"));
    multiColors.emplace_back(wxColour("PURPLE"));
    multiColors.emplace_back(wxColour("SALMON"));
    multiColors.emplace_back(wxColour("SEA GREEN"));
    multiColors.emplace_back(wxColour("SIENNA"));
    multiColors.emplace_back(wxColour("SKY BLUE"));
    multiColors.emplace_back(wxColour("SLATE BLUE"));
    multiColors.emplace_back(wxColour("SPRING GREEN"));
    multiColors.emplace_back(wxColour("STEEL BLUE"));
    multiColors.emplace_back(wxColour("TAN"));
    multiColors.emplace_back(wxColour("THISTLE"));
    multiColors.emplace_back(wxColour("TURQUOISE"));
    multiColors.emplace_back(wxColour("VIOLET"));
    multiColors.emplace_back(wxColour("VIOLET RED"));
    multiColors.emplace_back(wxColour("WHEAT"));
    multiColors.emplace_back(wxColour("YELLOW GREEN"));

    const int numColors = (number_of_colors < static_cast<int>(multiColors.size())) ? number_of_colors : static_cast<int>(multiColors.size());
    multiColors_.assign(multiColors.cbegin(), multiColors.cbegin() + numColors);
}

wxColour StationNode::GetNextColor() const
{
    if (currentColorIndex_ >= static_cast<int>(multiColors_.size()))
    {
        currentColorIndex_ = 0;
    }

    if (multiColors_.empty())
    {
        return drawStyle_.strokeColor_;
    }
    else
    {
        return multiColors_[currentColorIndex_++];
    }
}

void StationNode::Save(const H5::Group &g) const
{
    std::string utf8Title(title_.ToUTF8().data());
    if (g.nameExists(utf8Title))
    {
        H5Ldelete(g.getId(), utf8Title.data(), H5P_DEFAULT);
    }

    if (!g.nameExists(utf8Title))
    {
        H5::LinkCreatPropList lcpl;
        lcpl.setCharEncoding(H5T_CSET_UTF8);
        H5::Group cwg = g.createGroup(title_, lcpl);
        H5DB::SetAttribute(cwg, CommonDef::GetSpamDBNodeTypeAttrName(), GetTypeName());
        H5DB::SetAttribute(cwg, std::string("Current"), current_);
        H5DB::SetAttribute(cwg, CommonDef::GetStationTabAttrName(), tabContainerName_);
        H5DB::SetAttribute(cwg, std::string("DrawMode"), drawMode_);
        H5DB::Save(cwg, std::string("MultiColors"), multiColors_);
        SaveImage(cwg);
        ModelNode::Save(cwg);
    }
}

void StationNode::Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me)
{
    current_          = H5DB::GetAttribute<bool>(g, std::string("Current"));
    tabContainerName_ = H5DB::GetAttribute<std::string>(g, CommonDef::GetStationTabAttrName());
    drawMode_         = H5DB::GetAttribute<std::string>(g, std::string("DrawMode"));
    H5DB::Load(g, std::string("MultiColors"), multiColors_);
    ImportImage(g);
    ModelNode::Load(g, nf, me);
}

void StationNode::SaveImage(const H5::Group &cwg) const
{
    if (!img_.empty())
    {
        int dph = img_.depth();
        if (CV_8U == dph)
        {
            int cnl = img_.channels();
            switch (cnl)
            {
            case 1: SaveGrayScaleImage(cwg); break;
            case 3: SaveRGBImage(cwg);       break;
            case 4: SaveRGBAImage(cwg);      break;
            default: break;
            }
        }
    }
}

void StationNode::ImportImage(const H5::Group &cwg)
{
    if (cwg.nameExists("image") && H5IMis_image(cwg.getId(), "image")>0)
    {
        hsize_t  width         = 0;
        hsize_t  height        = 0;
        hsize_t  planes        = 0;
        char     interlace[32] = {0};
        hssize_t npals         = 0;
        
        auto hret = H5IMget_image_info(cwg.getId(), "image", &width, &height, &planes, interlace, &npals);
        if (hret >= 0 && width>0 && height>0 && planes>0)
        {
            if (1==planes)
            {
                std::vector<unsigned char> buf(width*height);
                hret = H5IMread_image(cwg.getId(), "image", buf.data());
                if (hret >= 0)
                {
                    img_.create(height, width, CV_8UC1);
                    for (int r = 0; r<height; ++r)
                    {
                        auto pRow = img_.data + r * img_.step1();
                        ::memcpy(pRow, buf.data() + r * width, width);
                    }
                }
            }
            else if (3 == planes && std::string("INTERLACE_PIXEL") == std::string(interlace))
            {
                std::vector<unsigned char> buf(width*height*3);
                hret = H5IMread_image(cwg.getId(), "image", buf.data());
                if (hret >= 0)
                {
                    img_.create(height, width, CV_8UC3);
                    for (int r = 0; r<height; ++r)
                    {
                        auto pDstRow = img_.data + r * img_.step1();
                        auto pSrcRow = buf.data() + r * width * 3;
                        for (int c = 0; c<width; ++c)
                        {
                            auto pSrcPixel = pSrcRow + c * 3;
                            auto pDstPixel = pDstRow + c * 3;

                            pDstPixel[0] = pSrcPixel[2];
                            pDstPixel[1] = pSrcPixel[1];
                            pDstPixel[2] = pSrcPixel[0];
                        }
                    }
                }
            }
        }
    }
}

void StationNode::SaveGrayScaleImage(const H5::Group &cwg) const
{
    int width = img_.cols;
    int height = img_.rows;
    std::vector<unsigned char> buf(width*height);

    for (int r=0; r<height; ++r)
    {
        auto pRow = img_.data + r * img_.step1();
        ::memcpy(buf.data()+r*width, pRow, width);
    }

    H5IMmake_image_8bit(cwg.getId(), "image", static_cast<hsize_t>(width), static_cast<hsize_t>(height), buf.data());
}

void StationNode::SaveRGBImage(const H5::Group &cwg) const
{
    int width = img_.cols;
    int height = img_.rows;
    std::vector<unsigned char> buf(width*height*3);

    for (int r = 0; r<height; ++r)
    {
        auto pSrcRow = img_.data + r * img_.step1();
        auto pDstRow = buf.data() + r * width *3;
        for (int c = 0; c<width; ++c)
        {
            auto pSrcPixel = pSrcRow + c * 3;
            auto pDstPixel = pDstRow + c * 3;

            pDstPixel[0] = pSrcPixel[2];
            pDstPixel[1] = pSrcPixel[1];
            pDstPixel[2] = pSrcPixel[0];
        }
    }

    H5IMmake_image_24bit(cwg.getId(), "image", static_cast<hsize_t>(width), static_cast<hsize_t>(height), "INTERLACE_PIXEL", buf.data());
}

void StationNode::SaveRGBAImage(const H5::Group &cwg) const
{
    int width = img_.cols;
    int height = img_.rows;
    std::vector<unsigned char> buf(width*height * 3);

    for (int r = 0; r<height; ++r)
    {
        auto pSrcRow = img_.data + r * img_.step1();
        auto pDstRow = buf.data() + r * width * 3;
        for (int c = 0; c<width; ++c)
        {
            auto pSrcPixel = pSrcRow + c * 4;
            auto pDstPixel = pDstRow + c * 3;

            pDstPixel[0] = pSrcPixel[2];
            pDstPixel[1] = pSrcPixel[1];
            pDstPixel[2] = pSrcPixel[0];
        }
    }

    H5IMmake_image_24bit(cwg.getId(), "image", static_cast<hsize_t>(width), static_cast<hsize_t>(height), "INTERLACE_PIXEL", buf.data());
}
