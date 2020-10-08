#include "precomp.hpp"
#include "region_bool.hpp"
#include "region_impl.hpp"
#include <utility.hpp>

namespace cv {
namespace mvlab {

struct RegionImpl::ErosTransPoint
{
    cv::Point point; /**< x and y coordinate of the given point */
    int erosTrans; /**< erosion-transform-value at (x,y) */
};

struct RegionImpl::RetGenerateSkeletonB
{
    int lmin; /**< length of shortest run within B */
    std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>> skeletonB; /**< list of skeleton-points and their erosion transform values */
};

struct RegionImpl::RetGenerateErosionTransformX
{
    cv::Ptr<RegionImpl> Xlmin; /**< stores X with all runs removed which are shorter than L^B_min. */
    cv::Mat erosTransXlmin; /**< erosion-transform-values of X_{L^B_min} stored in an array */
    cv::Point translation; /**< Xlmin got translated by this value */
};

struct RetGenerateErosionTransformX2
{
    cv::Ptr<RegionImpl> Xlmin; /**< stores X with all runs removed which are shorter than L^B_min. */
    cv::Mat erosTransXlmin; /**< erosion-transform-values of X_{L^B_min} with A stored in an array */
    cv::Mat erosTrans2Xlmin; /**< erosion-transform-values of X_{L^B_min} with A^t stored in an array */
    cv::Point translation; /**< Xlmin got translated by this value */
};

RegionImpl::RetGenerateSkeletonB const RegionImpl::generateSkeletonBtrans(const RegionImpl *SE, const cv::Point &transVector)
{
    RetGenerateSkeletonB skeletonBtrans;
    skeletonBtrans.lmin = std::numeric_limits<int>::max();
    skeletonBtrans.skeletonB.resize(SE->rgn_runs_.size());

    ErosTransPoint *pData = skeletonBtrans.skeletonB.data();
    for (auto & r : SE->rgn_runs_) {
        pData->point = transVector - r.start();
        pData->erosTrans = r.len();
        pData += 1;
        skeletonBtrans.lmin = std::min(r.len(), skeletonBtrans.lmin);
    }

    return skeletonBtrans;
}

RegionImpl::RetGenerateSkeletonB const RegionImpl::generateSkeletonB(const RegionImpl *SE, const cv::Point &transVector)
{
    RetGenerateSkeletonB skeletonB;
    skeletonB.lmin = std::numeric_limits<int>::max();
    skeletonB.skeletonB.resize(SE->rgn_runs_.size());

    ErosTransPoint *pData = skeletonB.skeletonB.data();
    for (const auto &r : SE->rgn_runs_)
    {
        pData->point = r.end() + transVector;
        pData->erosTrans = r.len();
        pData += 1;
        skeletonB.lmin = std::min(r.len(), skeletonB.lmin);
    }

    return skeletonB;
}

RegionImpl::RetGenerateErosionTransformX const RegionImpl::generateErosionTransformX(const RegionImpl *SE, const int lmin) const
{
    RetGenerateErosionTransformX erosionTransformX;
    WinP Xbbox = RegionImpl::BoundingBox();
    WinP Bbbox = SE->BoundingBox();
    cv::Point tr = -Xbbox.upperLeft() + cv::Point(Bbbox.size());
    erosionTransformX.erosTransXlmin.create(Xbbox.height() + 2 * (Bbbox.height() + 1), Xbbox.width() + 2 * (Bbbox.width() + 1), CV_16SC1);
    Util::SIMDZeroMemory(erosionTransformX.erosTransXlmin.data, erosionTransformX.erosTransXlmin.rows * erosionTransformX.erosTransXlmin.cols * sizeof(short));

    RunSequence dstRuns(rgn_runs_.size());
    RunSequence::pointer pResRun = dstRuns.data();

    for (const auto & r : rgn_runs_)
    {
        if (r.len() >= lmin)
        {
            short j = 1;
            int ycoord = r.start().y + tr.y;
            short *pRow = erosionTransformX.erosTransXlmin.ptr<short>(ycoord);
            for (int i = r.start().x + tr.x; i < r.start().x + tr.x + r.len(); ++i) 
            {
                pRow[i] = j;
                j++;
            }

            pResRun->row = ycoord;
            pResRun->colb = r.start().x + tr.x;
            pResRun->cole = pResRun->colb + r.len();
            pResRun->label = 0;
            pResRun += 1;
        }
    }

    dstRuns.resize(std::distance(dstRuns.data(), pResRun));
    erosionTransformX.translation = tr;
    erosionTransformX.Xlmin = makePtr<RegionImpl>(&dstRuns);

    return erosionTransformX;
}

RegionImpl::RetGenerateErosionTransformX const RegionImpl::generateErosionTransformX2(const RegionImpl *SE, const int lmin) const
{
    RetGenerateErosionTransformX erosionTransformX;

    WinP Xbbox = RegionImpl::BoundingBox();
    WinP Bbbox = SE->BoundingBox();
    cv::Point tr = -Xbbox.upperLeft() + cv::Point(Bbbox.size());
    erosionTransformX.erosTransXlmin.create(Xbbox.height() + 2 * (Bbbox.height() + 1), 2 * (Xbbox.width() + 2 * (Bbbox.width() + 1)), CV_16SC1);
    Util::SIMDZeroMemory(erosionTransformX.erosTransXlmin.data, erosionTransformX.erosTransXlmin.rows * erosionTransformX.erosTransXlmin.cols * sizeof(short));

    RunSequence dstRuns(rgn_runs_.size());
    RunSequence::pointer pResRun = dstRuns.data();

    for (const auto & r : rgn_runs_) {
        if (r.len() >= lmin) {
            short j = 1;
            short j2 = r.len();
            int ycoord = r.start().y + tr.y;
            short *pRow = erosionTransformX.erosTransXlmin.ptr<short>(ycoord);
            for (int i = r.start().x + tr.x; i < r.start().x + tr.x + r.len(); ++i) {
                pRow[i << 1] = j;
                pRow[(i << 1) + 1] = j2;
                j++;
                j2--;
            }

            pResRun->row = ycoord;
            pResRun->colb = r.start().x + tr.x;
            pResRun->cole = pResRun->colb + r.len();
            pResRun->label = 0;
            pResRun += 1;
        }
    }

    dstRuns.resize(std::distance(dstRuns.data(), pResRun));
    erosionTransformX.translation = tr;
    erosionTransformX.Xlmin = makePtr<RegionImpl>(&dstRuns);

    return erosionTransformX;
}

RegionImpl::RetGenerateErosionTransformX const RegionImpl::generateErosionTransformX2cut(const RegionImpl *SE, const int lmin, const int lmax) const
{
    RetGenerateErosionTransformX erosionTransformX;

    // -> creates the needed 2-dimensional array to store the erosion-transform of X
    WinP Xbbox = RegionImpl::BoundingBox();
    WinP Bbbox = SE->BoundingBox();
    cv::Point tr = -Xbbox.upperLeft() + cv::Point(Bbbox.size());
    erosionTransformX.erosTransXlmin.create(Xbbox.height() + 2 * (Bbbox.height() + 1), 2 * (Xbbox.width() + 2 * (Bbbox.width() + 1)), CV_16SC1);
    Util::SIMDZeroMemory(erosionTransformX.erosTransXlmin.data, erosionTransformX.erosTransXlmin.rows * erosionTransformX.erosTransXlmin.cols * sizeof(short));

    RunSequence dstRuns(rgn_runs_.size());
    RunSequence::pointer pResRun = dstRuns.data();

    for (const auto & r : rgn_runs_) {
        if (r.len() >= lmin) {
            short j = 1;
            short j2 = static_cast<short>(r.len());
            int ycoord = r.start().y + tr.y;
            short *pRow = erosionTransformX.erosTransXlmin.ptr<short>(ycoord);
            for (int i = r.start().x + tr.x; i < r.start().x + tr.x + r.len(); ++i) {
                pRow[i << 1] = j;
                pRow[(i << 1) + 1] = j2;
                j++;
                j2--;
            }

            if (r.len() >= lmax) {
                pResRun->row  = ycoord;
                pResRun->colb = r.start().x + tr.x + (lmax - 1);
                pResRun->cole = pResRun->colb + r.len() - (lmax - 1);
                pResRun->label = 0;
                pResRun += 1;
            }
        }
    }

    dstRuns.resize(std::distance(dstRuns.data(), pResRun));
    erosionTransformX.translation = tr;
    erosionTransformX.Xlmin = makePtr<RegionImpl>(&dstRuns);

    return erosionTransformX;
}

RegionImpl::RetGenerateErosionTransformX const RegionImpl::generateErosionTransformXcomp(const RegionImpl *SE, const int lmin) const
{
    RetGenerateErosionTransformX erosionTransformXc;

    WinP Xbbox = RegionImpl::BoundingBox();
    WinP Bbbox = SE->BoundingBox();
    cv::Point tr = -Xbbox.upperLeft() + cv::Point(Bbbox.size()*2);
    int Xcwidth = Xbbox.width() + 2 * Bbbox.width();
    int Xcheight = Xbbox.height() + 2 * Bbbox.height();
    erosionTransformXc.erosTransXlmin.create(Xcheight + 2 * (Bbbox.height() + 1), 2 * (Xcwidth + 2 * (Bbbox.width() + 1)), CV_16SC1);
    Util::SIMDZeroMemory(erosionTransformXc.erosTransXlmin.data, erosionTransformXc.erosTransXlmin.rows * erosionTransformXc.erosTransXlmin.cols * sizeof(short));

    int ycoordtr = Bbbox.height();
    int ycoord = Bbbox.height();
    int xcoord = Bbbox.width();

    erosionTransformXc.Xlmin = makePtr<RegionImpl>();
    RunSequence &dstRuns = const_cast<RunSequence &>(erosionTransformXc.Xlmin->rgn_runs_);
    dstRuns.reserve(rgn_runs_.size());

    for (auto & r : rgn_runs_) {
        int xcoordtr = r.start().x + tr.x;
        ycoordtr = r.start().y + tr.y;
        if (ycoord < ycoordtr) {
            if ((Xcwidth + Bbbox.width() - xcoord) >= lmin) {
                short j = 1;
                short j2 = Xcwidth + Bbbox.width() - xcoord;
                short *pRow = erosionTransformXc.erosTransXlmin.ptr<short>(ycoord);
                for (int x = xcoord; x < Xcwidth + Bbbox.width(); ++x) {
                    pRow[x << 1] = j;
                    pRow[(x << 1) + 1] = j2;
                    j++;
                    j2--;
                }
                dstRuns.emplace_back(cv::Point(xcoord, ycoord), Xcwidth + Bbbox.width() - xcoord);
            }

            if (Xcwidth >= lmin) {
                for (int y = ycoord + 1; y < ycoordtr; ++y) {
                    short j = 1;
                    short j2 = Xcwidth;
                    short *pRow = erosionTransformXc.erosTransXlmin.ptr<short>(y);
                    for (int x = Bbbox.width(); x < Xcwidth + Bbbox.width(); ++x) {
                        pRow[x << 1] = j;
                        pRow[(x << 1) + 1] = j2;
                        j++;
                        j2--;
                    }
                    dstRuns.emplace_back(cv::Point(Bbbox.width(), y), Xcwidth);
                }
            }

            if ((xcoordtr - Bbbox.width()) >= lmin) {
                short j = 1;
                short j2 = xcoordtr - Bbbox.width();
                short *pRow = erosionTransformXc.erosTransXlmin.ptr<short>(ycoordtr);
                for (int x = Bbbox.width(); x < xcoordtr; ++x) {
                    pRow[x << 1] = j;
                    pRow[(x << 1) + 1] = j2;
                    j++;
                    j2--;
                }
                dstRuns.emplace_back(cv::Point(Bbbox.width(), ycoordtr), xcoordtr - Bbbox.width());
            }

            xcoord = xcoordtr + r.len();
            ycoord = ycoordtr;
        } else {
            if ((xcoordtr - xcoord) >= lmin) {
                short j = 1;
                short j2 = xcoordtr - xcoord;
                short *pRow = erosionTransformXc.erosTransXlmin.ptr<short>(ycoordtr);
                for (int x = xcoord; x < xcoordtr; ++x) {
                    pRow[x << 1] = j;
                    pRow[(x << 1) + 1] = j2;
                    j++;
                    j2--;
                }
                dstRuns.emplace_back(cv::Point(xcoord, ycoordtr), xcoordtr - xcoord);
            }
            xcoord = xcoordtr + r.len();
        }
    }

    if ((Xcwidth + Bbbox.width() - xcoord) >= lmin) {
        short j = 1;
        short j2 = Xcwidth + Bbbox.width() - xcoord;
        short *pRow = erosionTransformXc.erosTransXlmin.ptr<short>(ycoordtr);
        for (int x = xcoord; x < Xcwidth + Bbbox.width(); ++x) {
            pRow[x << 1] = j;
            pRow[(x << 1) + 1] = j2;
            j++;
            j2--;
        }
        dstRuns.emplace_back(cv::Point(xcoord, ycoordtr), Xcwidth + Bbbox.width() - xcoord);
    }

    if (Xcwidth >= lmin) {
        for (int y = ycoordtr + 1; y < Xcheight + Bbbox.height(); ++y) {
            short j = 1;
            short j2 = Xcwidth;
            short *pRow = erosionTransformXc.erosTransXlmin.ptr<short>(y);
            for (int x = Bbbox.width(); x < Xcwidth + Bbbox.width(); ++x) {
                pRow[x << 1] = j;
                pRow[(x << 1) + 1] = j2;
                j++;
                j2--;
            }
            dstRuns.emplace_back(cv::Point(Bbbox.width(), y), Xcwidth);
        }
    }

    erosionTransformXc.translation = tr;
    return erosionTransformXc;
}

RegionImpl::RetGenerateErosionTransformX const RegionImpl::generateErosionTransformXcompcut(const RegionImpl *SE, const int lmin, const int lmax) const
{
    RetGenerateErosionTransformX erosionTransformXc;

    WinP Xbbox = RegionImpl::BoundingBox();
    WinP Bbbox = SE->BoundingBox();
    cv::Point tr = -Xbbox.upperLeft() + cv::Point(Bbbox.size() * 2);
    int Xcwidth = Xbbox.width() + 2 * Bbbox.width();
    int Xcheight = Xbbox.height() + 2 * Bbbox.height();
    erosionTransformXc.erosTransXlmin.create(Xcheight + 2 * (Bbbox.height() + 1), 2 * (Xcwidth + 2 * (Bbbox.width() + 1)), CV_16SC1);
    Util::SIMDZeroMemory(erosionTransformXc.erosTransXlmin.data, erosionTransformXc.erosTransXlmin.rows * erosionTransformXc.erosTransXlmin.cols * sizeof(short));

    int ycoord = Bbbox.height();
    int xcoord = Bbbox.width();
    int ycoordtr = Bbbox.height();

    erosionTransformXc.Xlmin = makePtr<RegionImpl>();
    RunSequence &dstRuns = const_cast<RunSequence &>(erosionTransformXc.Xlmin->rgn_runs_);
    dstRuns.reserve(rgn_runs_.size());

    for (auto & r : rgn_runs_) {
        int xcoordtr = r.start().x + tr.x;
        ycoordtr = r.start().y + tr.y;
        if (ycoord < ycoordtr) {
            if ((Xcwidth + Bbbox.width() - xcoord) >= lmin) {
                short j = 1;
                short j2 = Xcwidth + Bbbox.width() - xcoord;
                short *pRow = erosionTransformXc.erosTransXlmin.ptr<short>(ycoord);
                for (int x = xcoord; x < Xcwidth + Bbbox.width(); ++x) {
                    pRow[x << 1] = j;
                    pRow[(x << 1) + 1] = j2;
                    j++;
                    j2--;
                }
                if ((Xcwidth + Bbbox.width() - xcoord) >= lmax) {
                    dstRuns.emplace_back(cv::Point(xcoord + (lmax - 1), ycoord), Xcwidth + Bbbox.width() - xcoord - (lmax - 1));
                }
            }

            if (Xcwidth >= lmin) {
                for (int y = ycoord + 1; y < ycoordtr; ++y) {
                    short j = 1;
                    short j2 = Xcwidth;
                    short *pRow = erosionTransformXc.erosTransXlmin.ptr<short>(y);
                    for (int x = Bbbox.width(); x < Xcwidth + Bbbox.width(); ++x) {
                        pRow[x << 1] = j;
                        pRow[(x << 1) + 1] = j2;
                        j++;
                        j2--;
                    }
                    if (Xcwidth >= lmax) {
                        dstRuns.emplace_back(cv::Point(Bbbox.width() + (lmax - 1), y), Xcwidth - (lmax - 1));
                    }
                }
            }

            if ((xcoordtr - Bbbox.width()) >= lmin) {
                short j = 1;
                short j2 = xcoordtr - Bbbox.width();
                short *pRow = erosionTransformXc.erosTransXlmin.ptr<short>(ycoordtr);
                for (int x = Bbbox.width(); x < xcoordtr; ++x) {
                    pRow[x << 1] = j;
                    pRow[(x << 1) + 1] = j2;
                    j++;
                    j2--;
                }
                if ((xcoordtr - Bbbox.width()) >= lmax) {
                    dstRuns.emplace_back(cv::Point(Bbbox.width() + (lmax - 1), ycoordtr), xcoordtr - Bbbox.width() - (lmax - 1));
                }
            }

            xcoord = xcoordtr + r.len();
            ycoord = ycoordtr;

        }
        else {
            if ((xcoordtr - xcoord) >= lmin) {
                short j = 1;
                short j2 = xcoordtr - xcoord;
                short *pRow = erosionTransformXc.erosTransXlmin.ptr<short>(ycoordtr);
                for (int x = xcoord; x < xcoordtr; ++x) {
                    pRow[x << 1] = j;
                    pRow[(x << 1) + 1] = j2;
                    j++;
                    j2--;
                }

                if ((xcoordtr - xcoord) >= lmax) {
                    dstRuns.emplace_back(cv::Point(xcoord + (lmax - 1), ycoordtr), xcoordtr - xcoord - (lmax - 1));
                }
            }
            xcoord = xcoordtr + r.len();
        }
    }

    if ((Xcwidth + Bbbox.width() - xcoord) >= lmin) {
        short j = 1;
        short j2 = Xcwidth + Bbbox.width() - xcoord;
        short *pRow = erosionTransformXc.erosTransXlmin.ptr<short>(ycoordtr);
        for (int x = xcoord; x < Xcwidth + Bbbox.width(); ++x) {
            pRow[x << 1] = j;
            pRow[(x << 1) + 1] = j2;
            j++;
            j2--;
        }

        if ((Xcwidth + Bbbox.width() - xcoord) >= lmax) {
            dstRuns.emplace_back(cv::Point(xcoord + (lmax - 1), ycoordtr), Xcwidth + Bbbox.width() - xcoord - (lmax - 1));
        }
    }

    if (Xcwidth >= lmin) {
        for (int y = ycoordtr + 1; y < Xcheight + Bbbox.height(); ++y) {
            short j = 1;
            short j2 = Xcwidth;
            short *pRow = erosionTransformXc.erosTransXlmin.ptr<short>(y);
            for (int x = Bbbox.width(); x < Xcwidth + Bbbox.width(); ++x) {
                pRow[x << 1] = j;
                pRow[(x << 1) + 1] = j2;
                j++;
                j2--;
            }

            if (Xcwidth >= lmax) {
                dstRuns.emplace_back(cv::Point(Bbbox.width() + (lmax - 1), y), Xcwidth - (lmax - 1));
            }
        }
    }

    erosionTransformXc.translation = tr;
    return erosionTransformXc;
}

RegionImpl::RetGenerateErosionTransformX const RegionImpl::generateExtErosionTransformX(const RegionImpl *SE, const int lmin) const
{
    RetGenerateErosionTransformX erosionTransformX;
    WinP Xbbox = RegionImpl::BoundingBox();
    WinP Bbbox = SE->BoundingBox();
    cv::Point tr = -Xbbox.upperLeft() + cv::Point(Bbbox.size());

    cv::Mat &Xlmin = erosionTransformX.erosTransXlmin;
    Xlmin.create(Xbbox.height() + 2 * (Bbbox.height() + 1), 2 * (Xbbox.width() + 2 * (Bbbox.width() + 1)), CV_16SC1);
    std::memset(Xlmin.data, std::numeric_limits<int>::min(), Xlmin.rows * Xlmin.cols * sizeof(short));

    int ytemp = 0;
    int xcoord1 = 0;
    RunSequence dstRuns(rgn_runs_.size());
    RunSequence::pointer pResRun = dstRuns.data();

    for (auto & r : rgn_runs_) {
        int ycoordtr = r.start().y + tr.y;
        if (!(ytemp == ycoordtr)) {
            ytemp = ycoordtr;
            xcoord1 = 0;
        }
        int xcoordtr = r.start().x + tr.x;
        if (r.len() >= lmin) {
            short j = 1;
            short j2 = r.len();
            short *pRow = Xlmin.ptr<short>(ycoordtr);
            for (int i = xcoordtr; i < xcoordtr + r.len(); ++i) {
                pRow[i << 1] = j;
                pRow[(i << 1) + 1] = j2;
                j++;
                j2--;
            }
            j = 0;
            for (int i = xcoordtr - 1; xcoord1 <= i; --i) {
                pRow[i << 1] = j;
                j--;
            }

            pResRun->row = ycoordtr;
            pResRun->colb = xcoordtr;
            pResRun->cole = pResRun->colb + r.len();
            pResRun->label = 0;
            pResRun += 1;
            xcoord1 = xcoordtr + r.len();
        }
    }

    dstRuns.resize(std::distance(dstRuns.data(), pResRun));
    erosionTransformX.translation = tr;
    erosionTransformX.Xlmin = makePtr<RegionImpl>(&dstRuns);

    return erosionTransformX;
}

cv::Ptr<Region> RegionImpl::erode1(const cv::Ptr<Region> &structElement) const
{
    const cv::Ptr<RegionImpl> SE = structElement.dynamicCast<RegionImpl>();
    if (!SE || SE->Empty())
    {
        return (cv::Ptr<RegionImpl>)const_cast<RegionImpl *>(this)->shared_from_this();
    }

    cv::Point origTranslate = -SE->rgn_runs_.begin()->start();
    RetGenerateSkeletonB skelB = generateSkeletonB(SE, origTranslate);
    RetGenerateErosionTransformX erosTransX = generateErosionTransformX(SE, skelB.lmin);

    cv::Ptr<RegionImpl> Xlmin = erosTransX.Xlmin;
    const std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>> &skeletonB = skelB.skeletonB;

    int Diff = 0;
    cv::Ptr<RegionImpl> erodedRgn = makePtr<RegionImpl>();
    RunSequence &dstRuns = const_cast<RunSequence &>(erodedRgn->rgn_runs_);
    dstRuns.reserve(Xlmin->rgn_runs_.size());

    for (auto & r : Xlmin->rgn_runs_) {
        int xcoord = r.start().x;
        int eroStartx = r.start().x;
        int eroEndx = -1;
        int xend = r.start().x + r.len() - 1;
        while (xcoord <= xend)
        {
            bool breakl = false;
            std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>>::const_iterator iterSkelB = skeletonB.cbegin();
            while ((!breakl) && iterSkelB != skeletonB.cend())
            {
                short *pRow = erosTransX.erosTransXlmin.ptr<short>(iterSkelB->point.y + r.start().y);
                while ((xcoord <= xend) && ((Diff = iterSkelB->erosTrans - pRow[iterSkelB->point.x + xcoord]) > 0))
                {
                    breakl = true;
                    xcoord = xcoord + Diff;
                }
                iterSkelB += 1;
            }

            if (breakl) {
                if (eroEndx >= eroStartx)
                {
                    dstRuns.emplace_back(cv::Point(eroStartx - erosTransX.translation.x, r.start().y - erosTransX.translation.y), eroEndx - eroStartx + 1);
                }
                eroStartx = xcoord;
                eroEndx = -1;
            } else {
                eroEndx = xcoord;
                xcoord++;
            }
        }
        if (eroEndx >= eroStartx) {
            dstRuns.emplace_back(cv::Point(eroStartx - erosTransX.translation.x, r.start().y - erosTransX.translation.y), eroEndx - eroStartx + 1);
        }
    }

    for ( auto &r : dstRuns)
    {
        r.row += origTranslate.y;
        r.colb += origTranslate.x;
        r.cole += origTranslate.x;
    }

    return erodedRgn;
}

cv::Ptr<Region> RegionImpl::erode2(const cv::Ptr<Region> &structElement) const
{
    const cv::Ptr<RegionImpl> SE = structElement.dynamicCast<RegionImpl>();
    if (!SE || SE->Empty())
    {
        return (cv::Ptr<RegionImpl>)const_cast<RegionImpl *>(this)->shared_from_this();
    }

    cv::Point origTranslate = -SE->rgn_runs_.begin()->start();
    RetGenerateSkeletonB skelB = generateSkeletonB(SE, origTranslate);
    RetGenerateErosionTransformX erosTransX = generateErosionTransformX2(SE, skelB.lmin);

    cv::Ptr<RegionImpl> Xlmin = erosTransX.Xlmin;
    const std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>> &skeletonB = skelB.skeletonB;

    int Diff = 0;
    cv::Ptr<RegionImpl> erodedRgn = makePtr<RegionImpl>();
    RunSequence &dstRuns = const_cast<RunSequence &>(erodedRgn->rgn_runs_);
    dstRuns.reserve(Xlmin->rgn_runs_.size());

    for (auto & r : Xlmin->rgn_runs_) {
        int xcoord = r.start().x;
        int xend = r.start().x + r.len() - 1;
        while (xcoord <= xend) {
            bool breakl = false;
            std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>>::const_iterator iterSkelB = skeletonB.begin();
            while ((!breakl) && iterSkelB != skeletonB.end()) {
                short *pRow = erosTransX.erosTransXlmin.ptr<short>(iterSkelB->point.y + r.start().y);
                while ((xcoord <= xend) && ((Diff = iterSkelB->erosTrans - pRow[(iterSkelB->point.x + xcoord) << 1]) > 0)) {
                    breakl = true;
                    xcoord = xcoord + Diff;
                }
                iterSkelB++;
            }

            if (!breakl) {
                short minDist = std::numeric_limits<short>::max();
                for (std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>>::const_iterator iterSkel = skeletonB.begin(); iterSkel != skeletonB.end(); ++iterSkel) {
                    short *pRow = erosTransX.erosTransXlmin.ptr<short>(iterSkel->point.y + r.start().y);
                    minDist = std::min(minDist, pRow[((iterSkel->point.x + xcoord) << 1) + 1]);
                }

                dstRuns.emplace_back(cv::Point(xcoord - erosTransX.translation.x, r.start().y - erosTransX.translation.y), minDist);
                xcoord = xcoord + minDist + 1;
            }
        }
    }

    for (auto &r : dstRuns)
    {
        r.row += origTranslate.y;
        r.colb += origTranslate.x;
        r.cole += origTranslate.x;
    }

    return erodedRgn;
}

cv::Ptr<Region> RegionImpl::erode2cut(const cv::Ptr<Region> &structElement) const
{
    const cv::Ptr<RegionImpl> SE = structElement.dynamicCast<RegionImpl>();
    if (!SE || SE->Empty())
    {
        return (cv::Ptr<RegionImpl>)const_cast<RegionImpl *>(this)->shared_from_this();
    }

    int lmax = 0;
    cv::Point origTranslate;

    for (auto & r : SE->rgn_runs_) {
        if (r.len() > lmax) {
            lmax = r.len();
            origTranslate = -cv::Point(r.start().x + r.len() - 1, r.start().y);
        }
    }

    RetGenerateSkeletonB skelB = generateSkeletonB(SE, origTranslate);
    RetGenerateErosionTransformX erosTransX = generateErosionTransformX2cut(SE, skelB.lmin, lmax);

    cv::Ptr<RegionImpl> Xlmin = erosTransX.Xlmin;
    const std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>> &skeletonB = skelB.skeletonB;

    int Diff = 0;
    cv::Ptr<RegionImpl> erodedRgn = makePtr<RegionImpl>();
    RunSequence &dstRuns = const_cast<RunSequence &>(erodedRgn->rgn_runs_);
    dstRuns.reserve(Xlmin->rgn_runs_.size());

    for (auto & r : Xlmin->rgn_runs_) {
        int xcoord = r.start().x;
        int xend = r.start().x + r.len() - 1;
        while (xcoord <= xend) {
            bool breakl = false;
            std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>>::const_iterator iterSkelB = skeletonB.begin();
            while ((!breakl) && iterSkelB != skeletonB.end()) {
                short *pRow = erosTransX.erosTransXlmin.ptr<short>(iterSkelB->point.y + r.start().y);
                while ((xcoord <= xend) && ((Diff = iterSkelB->erosTrans - pRow[(iterSkelB->point.x + xcoord) << 1]) > 0)) {
                    breakl = true;
                    xcoord = xcoord + Diff;
                }
                iterSkelB++;
            }

            if (!breakl) {
                short minDist = std::numeric_limits<short>::max();
                for (std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>>::const_iterator iterSkel = skeletonB.begin(); iterSkel != skeletonB.end(); ++iterSkel) {
                    short *pRow = erosTransX.erosTransXlmin.ptr<short>(iterSkel->point.y + r.start().y);
                    minDist = std::min(minDist, pRow[((iterSkel->point.x + xcoord) << 1) + 1]);
                }

                dstRuns.emplace_back(cv::Point(xcoord - erosTransX.translation.x, r.start().y - erosTransX.translation.y), minDist);
                xcoord = xcoord + minDist + 1;
            }
        }
    }


    for (auto &r : dstRuns)
    {
        r.row += origTranslate.y;
        r.colb += origTranslate.x;
        r.cole += origTranslate.x;
    }

    return erodedRgn;
}

cv::Ptr<Region> RegionImpl::erode3(const cv::Ptr<Region> &structElement) const
{
    const cv::Ptr<RegionImpl> SE = structElement.dynamicCast<RegionImpl>();
    if (!SE || SE->Empty())
    {
        return (cv::Ptr<RegionImpl>)const_cast<RegionImpl *>(this)->shared_from_this();
    }

    cv::Point origTranslate = -SE->rgn_runs_.begin()->start();
    RetGenerateSkeletonB skelB = generateSkeletonB(SE, origTranslate);
    RetGenerateErosionTransformX erosTransX = generateExtErosionTransformX(SE, skelB.lmin);

    cv::Ptr<RegionImpl> Xlmin = erosTransX.Xlmin;
    const std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>> &skeletonB = skelB.skeletonB;

    cv::Ptr<RegionImpl> erodedRgn = makePtr<RegionImpl>();
    RunSequence &dstRuns = const_cast<RunSequence &>(erodedRgn->rgn_runs_);
    dstRuns.reserve(Xlmin->rgn_runs_.size());

    for (auto & r : Xlmin->rgn_runs_) {
        int xcoord = r.start().x;
        int xend = r.start().x + r.len() - 1;
        while (xcoord <= xend) {
            bool breakl = false;
            std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>>::const_iterator iterSkelB = skeletonB.begin();
            while ((!breakl) && iterSkelB != skeletonB.end()) {
                short *pRow = erosTransX.erosTransXlmin.ptr<short>(iterSkelB->point.y + r.start().y);
                int Diff = iterSkelB->erosTrans - pRow[(iterSkelB->point.x + xcoord) << 1];
                if (Diff > 0) {
                    breakl = true;
                    xcoord = xcoord + Diff;
                }
                iterSkelB++;
            }

            if (!breakl) {
                short minDist = std::numeric_limits<short>::max();
                for (std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>>::const_iterator iterSkel = skeletonB.begin(); iterSkel != skeletonB.end(); ++iterSkel) {
                    short *pRow = erosTransX.erosTransXlmin.ptr<short>(iterSkel->point.y + r.start().y);
                    minDist = std::min(minDist, pRow[((iterSkel->point.x + xcoord) << 1) + 1]);
                }

                dstRuns.emplace_back(cv::Point(xcoord - erosTransX.translation.x, r.start().y - erosTransX.translation.y), minDist);
                xcoord = xcoord + minDist + 1;
            }
        }
    }

    for (auto &r : dstRuns)
    {
        r.row += origTranslate.y;
        r.colb += origTranslate.x;
        r.cole += origTranslate.x;
    }

    return erodedRgn;
}

cv::Ptr<Region> RegionImpl::dilate(const cv::Ptr<Region> &structElement) const
{
    const cv::Ptr<RegionImpl> SE = structElement.dynamicCast<RegionImpl>();
    if (!SE || SE->Empty())
    {
        return (cv::Ptr<RegionImpl>)const_cast<RegionImpl *>(this)->shared_from_this();
    }

    cv::Point origTranslate = SE->rgn_runs_.begin()->start();
    RetGenerateSkeletonB skelBtrans = generateSkeletonBtrans(SE, origTranslate);
    RetGenerateErosionTransformX erosTransXc = generateErosionTransformXcomp(SE, skelBtrans.lmin);

    cv::Ptr<RegionImpl> Xclmin = erosTransXc.Xlmin;
    std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>> &skeletonBtrans = skelBtrans.skeletonB;

    int ytemp = 0;
    int firstrun = true;
    int runbegin = 0;

    cv::Ptr<RegionImpl> dilatedRgn = makePtr<RegionImpl>();
    RunSequence &dstRuns = const_cast<RunSequence &>(dilatedRgn->rgn_runs_);
    dstRuns.reserve(Xclmin->rgn_runs_.size());

    for (auto & r : Xclmin->rgn_runs_) {
        int xcoord = r.start().x;
        int xend = r.start().x + r.len() - 1;
        if (ytemp != r.start().y) {
            firstrun = true;
            ytemp = r.start().y;
        }
        while (xcoord <= xend) {
            bool breakl = false;
            std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>>::const_iterator iterSkelB = skeletonBtrans.begin();
            while ((!breakl) && iterSkelB != skeletonBtrans.end()) {
                int Diff = 0;
                short *pRow = erosTransXc.erosTransXlmin.ptr<short>(iterSkelB->point.y + r.start().y);
                while ((xcoord <= xend) && ((Diff = iterSkelB->erosTrans - pRow[(iterSkelB->point.x + xcoord) << 1]) > 0)) {
                    breakl = true;
                    xcoord = xcoord + Diff;
                }
                iterSkelB++;
            }

            if (!breakl) {
                short minDist = std::numeric_limits<short>::max();
                for (std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>>::const_iterator iterSkel = skeletonBtrans.begin(); iterSkel != skeletonBtrans.end(); ++iterSkel) {
                    short *pRow = erosTransXc.erosTransXlmin.ptr<short>(iterSkel->point.y + r.start().y);
                    minDist = std::min(minDist, pRow[((iterSkel->point.x + xcoord) << 1) + 1]);
                }
                if (!firstrun) {
                    dstRuns.emplace_back(cv::Point(runbegin - erosTransXc.translation.x, r.start().y - erosTransXc.translation.y), xcoord - runbegin);
                }
                runbegin = xcoord + minDist;
                firstrun = false;
                xcoord = xcoord + minDist + 1;
            }
        }
    }

    for (auto &r : dstRuns)
    {
        r.row += origTranslate.y;
        r.colb += origTranslate.x;
        r.cole += origTranslate.x;
    }

    return dilatedRgn;
}

cv::Ptr<Region> RegionImpl::dilatecut(const cv::Ptr<Region> &structElement) const
{
    const cv::Ptr<RegionImpl> SE = structElement.dynamicCast<RegionImpl>();
    if (!SE || SE->Empty())
    {
        return (cv::Ptr<RegionImpl>)const_cast<RegionImpl *>(this)->shared_from_this();
    }

    int lmax = 0;
    cv::Point origTranslate;

    for (auto & r : SE->rgn_runs_) {
        if (r.len() > lmax) {
            lmax = r.len();
            origTranslate = r.start();
        }
    }

    RetGenerateSkeletonB skelBtrans = generateSkeletonBtrans(SE, origTranslate);
    RetGenerateErosionTransformX erosTransXc = generateErosionTransformXcompcut(SE, skelBtrans.lmin, lmax);

    cv::Ptr<RegionImpl> Xclmin = erosTransXc.Xlmin;
    std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>> &skeletonBtrans = skelBtrans.skeletonB;

    int ytemp = 0;
    int firstrun = true;
    int runbegin = 0;

    cv::Ptr<RegionImpl> dilatedRgn = makePtr<RegionImpl>();
    RunSequence &dstRuns = const_cast<RunSequence &>(dilatedRgn->rgn_runs_);
    dstRuns.reserve(Xclmin->rgn_runs_.size());

    for (auto & r : Xclmin->rgn_runs_) {
        int xcoord = r.start().x;
        int xend = r.start().x + r.len() - 1;
        if (ytemp != r.start().y) {
            firstrun = true;
            ytemp = r.start().y;
        }
        while (xcoord <= xend) {
            bool breakl = false;
            std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>>::const_iterator iterSkelB = skeletonBtrans.begin();
            while ((!breakl) && iterSkelB != skeletonBtrans.end()) {
                int Diff = 0;
                short *pRow = erosTransXc.erosTransXlmin.ptr<short>(iterSkelB->point.y + r.start().y);
                while ((xcoord <= xend) && ((Diff = iterSkelB->erosTrans - pRow[(iterSkelB->point.x + xcoord) << 1]) > 0)) {
                    breakl = true;
                    xcoord = xcoord + Diff;
                }
                iterSkelB++;
            }

            if (!breakl) {
                short minDist = std::numeric_limits<short>::max();
                for (std::vector<ErosTransPoint, MyAlloc<ErosTransPoint>>::const_iterator iterSkel = skeletonBtrans.begin(); iterSkel != skeletonBtrans.end(); ++iterSkel) {
                    short *pRow = erosTransXc.erosTransXlmin.ptr<short>(iterSkel->point.y + r.start().y);
                    minDist = min(minDist, pRow[((iterSkel->point.x + xcoord) << 1) + 1]);
                }
                if (!firstrun) {
                    dstRuns.emplace_back(cv::Point(runbegin - erosTransXc.translation.x, r.start().y - erosTransXc.translation.y), xcoord - runbegin);
                }
                runbegin = xcoord + minDist;
                firstrun = false;
                xcoord = xcoord + minDist + 1;
            }
        }
    }

    for (auto &r : dstRuns)
    {
        r.row += origTranslate.y;
        r.colb += origTranslate.x;
        r.cole += origTranslate.x;
    }

    return dilatedRgn;
}

class RLEDilation {
private:
    const RunSequence *const Z_;
    const RunSequence *const SE_;

public:
    RunSequence uRuns;

public:
    void operator()(const tbb::blocked_range<int>& r)
    {
        RunSequence dstRuns(Z_->size());
        for (int i = r.begin(); i != r.end(); ++i)
        {
            RunLength *pRL = dstRuns.data();
            const RunLength &s = (*SE_)[i];
            for (const auto & z : *Z_)
            {
                pRL->row   = s.row + z.row;
                pRL->colb  = s.colb + z.colb;
                pRL->cole  = s.cole + z.cole - 1;
                pRL->label = 0;
                pRL += 1;
            }

            if (uRuns.empty())
            {
                uRuns.swap(dstRuns);
                RunSequence t(Z_->size());
                dstRuns.swap(t);
            }
            else
            {
                RunSequence rRuns;
                RegionUnion2Op unionOp;
                unionOp.Do(dstRuns, uRuns, rRuns);
                rRuns.swap(uRuns);
            }
        }
    }

    RLEDilation(RLEDilation& x, tbb::split) : Z_(x.Z_), SE_(x.SE_) {}
    void join(const RLEDilation& y)
    {
        RunSequence rRuns;
        RegionUnion2Op unionOp;
        unionOp.Do(uRuns, y.uRuns, rRuns);
        rRuns.swap(uRuns);
    }

    RLEDilation(const RunSequence *const Z, const RunSequence *const SE)
        : Z_(Z), SE_(SE)
    {}
};

class SIMDDilation {
private:
    const RunSequence *const Z_;
    const RunSequence *const SE_;

public:
    RunSequence uRuns;

public:
    void operator()(const tbb::blocked_range<int>& br)
    {
        RunSequence dstRuns(Z_->size());
        constexpr int simdSize = 8;
        const int numRuns = static_cast<int>(Z_->size());
        const int regularNumRuns = numRuns & (-simdSize);
        vcl::Vec8i label(0);

        for (int i = br.begin(); i != br.end(); ++i)
        {
            const RunLength &s = (*SE_)[i];
            vcl::Vec8i sRow(s.row), sColb(s.colb), sCole(s.cole-1);

            int r = 0;
            RunLength *pRL = dstRuns.data();
            const RunLength *pZ = Z_->data();
            for (; r < regularNumRuns; r += simdSize)
            {
                vcl::Vec8i v1, v2, v3, v4;
                v1.load(reinterpret_cast<const int *>(pZ));
                v2.load(reinterpret_cast<const int *>(pZ + simdSize / 4));
                v3.load(reinterpret_cast<const int *>(pZ + simdSize / 2));
                v4.load(reinterpret_cast<const int *>(pZ + 3 * simdSize / 4));

                vcl::Vec8i r0 = vcl::blend8<0, 4, 8, 12, 1, 5, 9, 13>(v1, v2);
                vcl::Vec8i r1 = vcl::blend8<2, 6, 10, 14, 3, 7, 11, 15>(v1, v2);
                vcl::Vec8i r2 = vcl::blend8<0, 4, 8, 12, 1, 5, 9, 13>(v3, v4);
                vcl::Vec8i r3 = vcl::blend8<2, 6, 10, 14, 3, 7, 11, 15>(v3, v4);
                vcl::Vec8i zRow = vcl::blend8<0, 1, 2, 3, 8, 9, 10, 11>(r0, r2);
                vcl::Vec8i zColb = vcl::blend8<4, 5, 6, 7, 12, 13, 14, 15>(r0, r2);
                vcl::Vec8i zCole = vcl::blend8<0, 1, 2, 3, 8, 9, 10, 11>(r1, r3);

                vcl::Vec8i rRow = sRow + zRow;
                vcl::Vec8i rColb = sColb + zColb;

                r0 = vcl::blend8<0, 1, 2, 3, 8, 9, 10, 11>(rRow, rColb);
                r1 = vcl::blend8<0, 1, 2, 3, 8, 9, 10, 11>(zCole, label);
                r2 = vcl::blend8<4, 5, 6, 7, 12, 13, 14, 15>(rRow, rColb);
                r3 = vcl::blend8<4, 5, 6, 7, 12, 13, 14, 15>(zCole, label);

                v1 = vcl::blend8<0, 4, 8, 12, 1, 5, 9, 13>(r0, r1);
                v2 = vcl::blend8<2, 6, 10, 14, 3, 7, 11, 15>(r0, r1);
                v3 = vcl::blend8<0, 4, 8, 12, 1, 5, 9, 13>(r2, r3);
                v4 = vcl::blend8<2, 6, 10, 14, 3, 7, 11, 15>(r2, r3);

                v1.store(pRL);
                v2.store(pRL + simdSize / 4);
                v3.store(pRL + simdSize / 2);
                v4.store(pRL + 3 * simdSize / 4);

                pRL += simdSize;
                pZ += simdSize;
            }

            for (; r < numRuns; ++r)
            {
                pRL->row = s.row + pZ->row;
                pRL->colb = s.colb + pZ->colb;
                pRL->cole = s.cole + pZ->cole - 1;
                pRL->label = 0;
                pRL += 1;
                pZ += 1;
            }

            if (uRuns.empty())
            {
                uRuns.swap(dstRuns);
                RunSequence t(Z_->size());
                dstRuns.swap(t);
            }
            else
            {
                RunSequence rRuns;
                RegionUnion2Op unionOp;
                unionOp.Do(dstRuns, uRuns, rRuns);
                rRuns.swap(uRuns);
            }
        }
    }

    SIMDDilation(SIMDDilation& x, tbb::split) : Z_(x.Z_), SE_(x.SE_) {}
    void join(const SIMDDilation& y)
    {
        RunSequence rRuns;
        RegionUnion2Op unionOp;
        unionOp.Do(uRuns, y.uRuns, rRuns);
        rRuns.swap(uRuns);
    }

    SIMDDilation(const RunSequence *const Z, const RunSequence *const SE)
        : Z_(Z), SE_(SE)
    {}
};

class RLEErosion {
private:
    const RunSequence *const Z_;
    const RunSequence *const SE_;
    bool first_ = true;

public:
    RunSequence uRuns;

public:
    void operator()(const tbb::blocked_range<int>& r)
    {
        RunSequence lRuns(Z_->size());
        for (int i = r.begin(); i != r.end(); ++i)
        {
            lRuns.resize(Z_->size());
            RunLength *pRL = lRuns.data();
            const RunLength &s = (*SE_)[i];
            for (const auto & z : *Z_)
            {
                if (s.len() <= z.len())
                {
                    pRL->row = z.row - s.row;
                    pRL->colb = z.colb - s.colb;
                    pRL->cole = z.cole - s.cole + 1;
                    pRL->label = 0;
                    pRL += 1;
                }
            }

            lRuns.resize(std::distance(lRuns.data(), pRL));

            if (first_)
            {
                first_ = false;
                uRuns.swap(lRuns);
                RunSequence t(Z_->size());
                lRuns.swap(t);
            }
            else
            {
                RegionIntersectionOp interOp;
                RunSequence rRuns = interOp.Do(lRuns, uRuns);
                rRuns.swap(uRuns);
            }
        }
    }

    RLEErosion(RLEErosion& x, tbb::split) : Z_(x.Z_), SE_(x.SE_) {}
    void join(const RLEErosion& y)
    {
        if (first_)
        {
            first_ = false;
            uRuns = y.uRuns;
        }
        else
        {
            RegionIntersectionOp interOp;
            RunSequence rRuns = interOp.Do(uRuns, y.uRuns);
            rRuns.swap(uRuns);
        }
    }

    RLEErosion(const RunSequence *const Z, const RunSequence *const SE)
        : Z_(Z), SE_(SE)
    {}
};

cv::Ptr<Region> RegionImpl::rleErosion(const cv::Ptr<Region> &structElement) const
{
    const cv::Ptr<RegionImpl> SE = structElement.dynamicCast<RegionImpl>();
    if (!SE || SE->Empty())
    {
        return (cv::Ptr<RegionImpl>)const_cast<RegionImpl *>(this)->shared_from_this();
    }

    RLEErosion rleE(&rgn_runs_, &(SE->rgn_runs_));
    if (SE->rgn_runs_.size() > 1)
    {
        tbb::parallel_reduce(tbb::blocked_range<int>(0, static_cast<int>(SE->rgn_runs_.size())), rleE);
    }
    else
    {
        rleE(tbb::blocked_range<int>(0, 1));
    }

    return makePtr<RegionImpl>(&rleE.uRuns);
}

cv::Ptr<Region> RegionImpl::rleDilation(const cv::Ptr<Region> &structElement) const
{
    const cv::Ptr<RegionImpl> SE = structElement.dynamicCast<RegionImpl>();
    if (!SE || SE->Empty())
    {
        return (cv::Ptr<RegionImpl>)const_cast<RegionImpl *>(this)->shared_from_this();
    }

    RLEDilation rleD(&rgn_runs_, &(SE->rgn_runs_));
    if (SE->rgn_runs_.size() > 1)
    {
        tbb::parallel_reduce(tbb::blocked_range<int>(0, static_cast<int>(SE->rgn_runs_.size())), rleD);
    }
    else
    {
        rleD(tbb::blocked_range<int>(0, 1));
    }

    return makePtr<RegionImpl>(&rleD.uRuns);
}

cv::Ptr<Region> RegionImpl::rleCompDilation(const cv::Ptr<Region> &structElement) const
{
    const cv::Ptr<RegionImpl> SE = structElement.dynamicCast<RegionImpl>();
    if (!SE || SE->Empty() || RegionImpl::Empty())
    {
        return (cv::Ptr<RegionImpl>)const_cast<RegionImpl *>(this)->shared_from_this();
    }

    constexpr int negInf = std::numeric_limits<int>::min() / 4;
    constexpr int posInf = std::numeric_limits<int>::max() / 4;
    const int compHeight = structElement->BoundingBox().height + 8;
    const cv::Point tlPt{ negInf, rgn_runs_.front().row - compHeight };
    const cv::Point brPt{ posInf, rgn_runs_.back().row + compHeight };

    RegionComplementOp compOp;
    RunSequence runs1 = compOp.Do1(rgn_runs_, tlPt, brPt);

    RLEErosion rleE(&runs1, &(SE->rgn_runs_));
    if (SE->rgn_runs_.size() > 1)
    {
        tbb::parallel_reduce(tbb::blocked_range<int>(0, static_cast<int>(SE->rgn_runs_.size())), rleE);
    }
    else
    {
        rleE(tbb::blocked_range<int>(0, 1));
    }

    RunSequence runs2 = compOp.Do2(rleE.uRuns);
    return makePtr<RegionImpl>(&runs2);
}

cv::Ptr<Region> RegionImpl::Dilation(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts) const
{
    if (opts)
    {
        cv::String method = opts->GetString("Method");
        if ("dilate" == method)
        {
            return dilate(structElement);
        }
        else if ("dilatecut" == method)
        {
            return dilatecut(structElement);
        }
        else if ("RLE" == method)
        {
            return rleDilation(structElement);
        }
        else
        {
            return rleCompDilation(structElement);
        }
    }

    return rleCompDilation(structElement);
}

cv::Ptr<Region> RegionImpl::Erosion(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts) const
{
    if (opts)
    {
        cv::String method = opts->GetString("Method");
        if ("erode1" == method)
        {
            return erode1(structElement);
        }
        else if ("erode2" == method)
        {
            return erode2(structElement);
        }
        else if ("erode3" == method)
        {
            return erode3(structElement);
        }
        else if ("erode2cut" == method)
        {
            return erode2cut(structElement);
        }
        else
        {
            return rleErosion(structElement);
        }
    }

    return rleErosion(structElement);
}

cv::Ptr<Region> RegionImpl::Opening(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts) const
{
    return Erosion(structElement)->Dilation(structElement);
}

cv::Ptr<Region> RegionImpl::Closing(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts) const
{
    return Dilation(structElement)->Erosion(structElement);
}

}
}
