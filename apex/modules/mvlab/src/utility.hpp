#ifndef __OPENCV_MVLAB_UTILITY_HPP__
#define __OPENCV_MVLAB_UTILITY_HPP__

#include <2geom/2geom.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/mvlab.hpp>
#include <boost/core/noncopyable.hpp>

namespace cv {
namespace mvlab {

class Util : private boost::noncopyable
{
public:
    Util() = delete;

public:
    static std::vector<double> GetDashesPattern(const int bls, const double lineWidth);
    static cv::Mat PathToMask(const Geom::PathVector &pv, const cv::Size &sz);
    static cv::Mat PathToMask(const Geom::PathVector &pv, const cv::Size &sz, UScalableUCharSequence &buf);
    static inline float constrainAngle(float x);
    static inline float square(float x) { return x * x; }
    static inline double square(double x) { return x * x; }
    static inline float rad(const float angle) { return angle * static_cast<float>(CV_PI) / 180.f; }
    static inline double rad(const double angle) { return angle * CV_PI / 180.; }
    static inline float deg(const float angle) { return angle * 180.f / static_cast<float>(CV_PI); }
    static inline double deg(const double angle) { return angle * 180. / CV_PI; }
    static inline float dist(const cv::Point2f *p0, const cv::Point2f *p1);
    static inline float dist(const cv::Point2f &p0, const cv::Point2f &p1);
    static inline cv::Point2f midPoint(const cv::Point2f *p0, const cv::Point2f *p1);
    static inline cv::Point2f interPoint(const float t, const cv::Point2f *p0, const cv::Point2f *p1);
    static inline cv::Point2f interPoint(const float t, const cv::Point2f &p0, const cv::Point2f &p1);
    static inline bool nearPoint(const cv::Point2f *p0, const cv::Point2f *p1, const float tol);
    static inline bool farPoint(const cv::Point2f *p0, const cv::Point2f *p1, const float tol = G_F_TOL);
    static inline bool farPoint(const cv::Point2f &p0, const cv::Point2f &p1, const float tol = G_F_TOL);
    static inline bool nearPoint(const cv::Point2f &p0, const cv::Point2f &p1, const float tol = G_F_TOL);
    static inline cv::Point changedToFixed(const cv::Point2f &point);
    static inline int isLeft(const cv::Point &P0, const cv::Point &P1, const cv::Point &P2);
    static inline float isLeft(const cv::Point2f &P0, const cv::Point2f &P1, const cv::Point2f &P2);
    static inline int isTolLeft(const cv::Point2f &P0, const cv::Point2f &P1, const cv::Point2f &P2, const float tol = G_F_TOL);
    static inline int isLeft(const int P0x, const int P0y, const int P1x, const int P1y, const int P2x, const int P2y);
    static inline vcl::Vec8f isLeft(const vcl::Vec8f &P0x, const vcl::Vec8f &P0y, const vcl::Vec8f &P1x, const vcl::Vec8f &P1y, const vcl::Vec8f &P2x, const vcl::Vec8f &P2y);
    static int CheckSaveParameters(const cv::String &fileName, const cv::Ptr<Dict> &opts, cv::String &errMsg);
    static int CheckLoadParameters(const cv::String &fileName, const cv::Ptr<Dict> &opts, cv::String &formatHint, cv::String &errMsg);
    static int CheckCompressLoadParameters(const cv::String &fileName, const cv::Ptr<Dict> &opts, cv::String &formatHint, cv::String &errMsg);
    static void SIMDZeroMemory(void *dest, const int sz);
};

class WinP
{
public:
    WinP() {}
    WinP(cv::Point const &ul, cv::Point const &lr) : ul_(ul), lr_(lr) {}
    WinP(int ulX, int ulY, int lrX, int lrY) : ul_(ulX, ulY), lr_(lrX, lrY) {}
    WinP(const cv::Rect &rc) : ul_(rc.tl()), lr_(rc.br() + cv::Point(-1, -1)) {}

    cv::Point const & upperLeft() const { return ul_; }
    cv::Point const & lowerRight() const { return lr_; }

    int width() const { return (lr_.x - ul_.x + 1); }
    int height() const { return (lr_.y - ul_.y + 1); }
    cv::Size size() const { return { width(), height() }; }
    bool includes(cv::Point const & pt) const { return pt.x >= ul_.x && pt.x <= lr_.x && pt.y >= ul_.y && pt.y <= lr_.y; }
    bool includes(WinP const & rhs) const { return this->includes(rhs.upperLeft()) && this->includes(rhs.lowerRight()); }

    template<typename OutIt>
    void corners(OutIt res) const
    {
        *res++ = this->upperLeft();
        *res++ = cv::Point(this->lowerRight().x, this->upperLeft().y);
        *res++ = this->lowerRight();
        *res++ = cv::Point(this->upperLeft().x, this->lowerRight().y);
    }

    bool operator==(WinP const & rhs) const { return this->upperLeft() == rhs.upperLeft() && this->lowerRight() == rhs.lowerRight(); }
    bool operator!=(WinP const & rhs) const { return !(*this == rhs); }

    WinP &translate(cv::Point const & v) { ul_ += v; lr_ += v; return *this; }
    WinP const getTranslate(cv::Point const & v) const { WinP r(*this); return r.translate(v); }
    bool clip(WinP const & other)
    {
        cv::Point ul(std::max(this->ul_.x, other.ul_.x), std::max(this->ul_.y, other.ul_.y));
        cv::Point lr(std::min(this->lr_.x, other.lr_.x), std::min(this->lr_.y, other.lr_.y));
        if (ul.x <= lr.x && ul.y <= lr.y)
        {
            this->ul_ = ul;
            this->lr_ = lr;
            return true;
        }
        else
        {
            return false;
        }
    }

private:
    cv::Point ul_;
    cv::Point lr_;
};

template<typename _Tp, size_t fixed_size = 1024 / sizeof(_Tp) + 8>
class AdaptBuffer
{
public:
    typedef _Tp value_type;

    //! the default constructor
    AdaptBuffer();
    //! constructor taking the real buffer size
    explicit AdaptBuffer(size_t _size);

    //! the copy constructor
    AdaptBuffer(const AdaptBuffer<_Tp, fixed_size>& buf);
    //! the assignment operator
    AdaptBuffer<_Tp, fixed_size>& operator = (const AdaptBuffer<_Tp, fixed_size>& buf);

    //! destructor. calls deallocate()
    ~AdaptBuffer();

    //! allocates the new buffer of size _size. if the _size is small enough, stack-allocated buffer is used
    void allocate(size_t _size);
    //! deallocates the buffer if it was dynamically allocated
    void deallocate();
    //! resizes the buffer and preserves the content
    void resize(size_t _size);
    //! returns the current buffer size
    size_t size() const;
    //! returns pointer to the real buffer, stack-allocated or heap-allocated
    inline _Tp* data() { return ptr; }
    //! returns read-only pointer to the real buffer, stack-allocated or heap-allocated
    inline const _Tp* data() const { return ptr; }

    inline _Tp* begin() noexcept { return ptr; }
    inline const _Tp* begin() const noexcept { return ptr; }
    inline _Tp* end() noexcept { return ptr + sz; }
    inline const _Tp* end() const noexcept { return ptr + sz; }
    inline const _Tp* cbegin() const noexcept { return ptr; }
    inline const _Tp* cend() const noexcept { return ptr + sz; }

    //! returns a reference to the element at specified location. No bounds checking is performed in Release builds.
    inline _Tp& operator[] (size_t i) { CV_DbgCheckLT(i, sz, "out of range"); return ptr[i]; }
    //! returns a reference to the element at specified location. No bounds checking is performed in Release builds.
    inline const _Tp& operator[] (size_t i) const { CV_DbgCheckLT(i, sz, "out of range"); return ptr[i]; }

protected:
    //! pointer to the real buffer, can point to buf if the buffer is small enough
    _Tp* ptr;
    //! size of the real buffer
    size_t sz;
    //! pre-allocated buffer. At least 1 element to confirm C++ standard requirements
    _Tp buf[(fixed_size > 0) ? fixed_size : 1];
};

template<typename TAngle>
struct AngleRange
{
    AngleRange(const TAngle s, const TAngle e)
        : start(normalize(s)), end(normalize(e))
    {
    }

    TAngle normalize(const TAngle a)
    {
        TAngle angle = a;
        while (angle < -180) angle += 360;
        while (angle > 180) angle -= 360;
        return angle;
    }

    bool contains(const TAngle a)
    {
        TAngle na = normalize(a);
        if (start < end) {
            return !(na > end || na < start);
        }
        else {
            return !(na > end && na < start);
        }
    }

    bool between(const TAngle a)
    {
        TAngle na = normalize(a);
        if (start < end) {
            return na < end && na > start;
        }
        else {
            return na < end || na > start;
        }
    }

    TAngle start;
    TAngle end;
};

struct OutsideImageBox
{
    OutsideImageBox(const int w, const int h) : width(w), height(h) {}
    bool operator()(const cv::Point &point)
    {
        if (point.x < 0 || point.x >= width || point.y < 0 || point.y >= height)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    const int width;
    const int height;
};

struct OutsideRectangle
{
    OutsideRectangle(const int l, const int r, const int t, const int b) : left(l), right(r), top(t), bottom(b) {}
    bool operator()(const cv::Point &point)
    {
        if (point.x < left || point.x > right || point.y < top || point.y > bottom)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    const int left;
    const int right;
    const int top;
    const int bottom;
};

/////////////////////////////// AdaptBuffer implementation ////////////////////////////////////////

template<typename _Tp, size_t fixed_size> inline
AdaptBuffer<_Tp, fixed_size>::AdaptBuffer()
{
    ptr = buf;
    sz = fixed_size;
}

template<typename _Tp, size_t fixed_size> inline
AdaptBuffer<_Tp, fixed_size>::AdaptBuffer(size_t _size)
{
    ptr = buf;
    sz = fixed_size;
    allocate(_size);
}

template<typename _Tp, size_t fixed_size> inline
AdaptBuffer<_Tp, fixed_size>::AdaptBuffer(const AdaptBuffer<_Tp, fixed_size>& abuf)
{
    ptr = buf;
    sz = fixed_size;
    allocate(abuf.size());
    for (size_t i = 0; i < sz; i++)
        ptr[i] = abuf.ptr[i];
}

template<typename _Tp, size_t fixed_size> inline AdaptBuffer<_Tp, fixed_size>&
AdaptBuffer<_Tp, fixed_size>::operator = (const AdaptBuffer<_Tp, fixed_size>& abuf)
{
    if (this != &abuf)
    {
        deallocate();
        allocate(abuf.size());
        for (size_t i = 0; i < sz; i++)
            ptr[i] = abuf.ptr[i];
    }
    return *this;
}

template<typename _Tp, size_t fixed_size> inline
AdaptBuffer<_Tp, fixed_size>::~AdaptBuffer()
{
    deallocate();
}

template<typename _Tp, size_t fixed_size> inline void
AdaptBuffer<_Tp, fixed_size>::allocate(size_t _size)
{
    if (_size <= sz)
    {
        sz = _size;
        return;
    }
    deallocate();
    sz = _size;
    if (_size > fixed_size)
    {
        ptr = static_cast<_Tp*>(::mi_malloc(_size * sizeof(_Tp)));
    }
}

template<typename _Tp, size_t fixed_size> inline void
AdaptBuffer<_Tp, fixed_size>::deallocate()
{
    if (ptr != buf)
    {
        ::mi_free(ptr);
        ptr = buf;
        sz = fixed_size;
    }
}

template<typename _Tp, size_t fixed_size> inline void
AdaptBuffer<_Tp, fixed_size>::resize(size_t _size)
{
    if (_size <= sz)
    {
        sz = _size;
        return;
    }
    size_t i, prevsize = sz, minsize = MIN(prevsize, _size);
    _Tp* prevptr = ptr;

    ptr = _size > fixed_size ? static_cast<_Tp*>(::mi_malloc(_size * sizeof(_Tp))) : buf;
    sz = _size;

    if (ptr != prevptr)
        for (i = 0; i < minsize; i++)
            ptr[i] = prevptr[i];
    for (i = prevsize; i < _size; i++)
        ptr[i] = _Tp();

    if (prevptr != buf)
        ::mi_free(prevptr);
}

template<typename _Tp, size_t fixed_size> inline size_t
AdaptBuffer<_Tp, fixed_size>::size() const
{
    return sz;
}

inline float Util::constrainAngle(float x)
{
    x = std::fmod(x, 360.f);
    if (x < 0)
        x += 360;
    return x;
}

inline float Util::dist(const cv::Point2f *p0, const cv::Point2f *p1)
{
    return std::sqrtf(square(p1->x-p0->x) + square(p1->y - p0->y));
}

inline float Util::dist(const cv::Point2f &p0, const cv::Point2f &p1)
{
    return std::sqrtf(square(p1.x - p0.x) + square(p1.y - p0.y));
}

inline cv::Point2f Util::midPoint(const cv::Point2f *p0, const cv::Point2f *p1)
{
    return cv::Point2f((p0->x + p1->x) / 2, (p0->y + p1->y) / 2);
}

inline cv::Point2f Util::interPoint(const float t, const cv::Point2f *p0, const cv::Point2f *p1)
{
    return (1 - t) * (*p0) + t * (*p1);
}

inline cv::Point2f Util::interPoint(const float t, const cv::Point2f &p0, const cv::Point2f &p1)
{
    return (1 - t) * p0 + t * p1;
}

inline bool Util::nearPoint(const cv::Point2f *p0, const cv::Point2f *p1, const float tol)
{
    return (square(p1->x - p0->x) + square(p1->y - p0->y)) < tol;
}

inline bool Util::farPoint(const cv::Point2f *p0, const cv::Point2f *p1, const float tol)
{
    return (square(p1->x - p0->x) + square(p1->y - p0->y)) > tol;
}

inline bool Util::farPoint(const cv::Point2f &p0, const cv::Point2f &p1, const float tol)
{
    const cv::Point2f dxy = p0 - p1;
    return dxy.dot(dxy) > tol;
}

inline bool Util::nearPoint(const cv::Point2f &p0, const cv::Point2f &p1, const float tol)
{
    const cv::Point2f dxy = p0 - p1;
    return dxy.dot(dxy) < tol;
}

inline cv::Point Util::changedToFixed(const cv::Point2f &point)
{
    return cv::Point(cvRound(point.x * F_XY_ONE), cvRound(point.y * F_XY_ONE));
}

inline int Util::isLeft(const cv::Point &P0, const cv::Point &P1, const cv::Point &P2)
{
    return (P2.x - P0.x) * (P1.y - P0.y) - (P1.x - P0.x) * (P2.y - P0.y);
}

inline float Util::isLeft(const cv::Point2f &P0, const cv::Point2f &P1, const cv::Point2f &P2)
{
    return (P2.x - P0.x) * (P1.y - P0.y) - (P1.x - P0.x) * (P2.y - P0.y);
}

inline int Util::isTolLeft(const cv::Point2f &P0, const cv::Point2f &P1, const cv::Point2f &P2, const float tol)
{
    const float dx = P1.x - P0.x;
    const float dy = P1.y - P0.y;
    const float sxy = dx * dx + dy * dy;
    const float a = (P2.x - P0.x) * dy - dx * (P2.y - P0.y);
    const float aa = a*a;
    if (aa > (tol*sxy))
    {
        return a > 0 ? 1 : -1;
    }
    else
    {
        return 0;
    }
}

inline int Util::isLeft(const int P0x, const int P0y, const int P1x, const int P1y, const int P2x, const int P2y)
{
    return (P2x - P0x) * (P1y - P0y) - (P1x - P0x) * (P2y - P0y);
}

inline vcl::Vec8f Util::isLeft(const vcl::Vec8f &P0x, const vcl::Vec8f &P0y,
    const vcl::Vec8f &P1x, const vcl::Vec8f &P1y,
    const vcl::Vec8f &P2x, const vcl::Vec8f &P2y)
{
    return (P2x - P0x) * (P1y - P0y) - (P1x - P0x) * (P2y - P0y);
}

template <class T>
int WriteToFile(const T &c, const char *label, const cv::String &fileName, const cv::Ptr<Dict> &opts, cv::String &errMsg)
{
    try
    {
        errMsg.resize(0);
        int r = Util::CheckSaveParameters(fileName, opts, errMsg);
        if (MLR_SUCCESS != r)
        {
            return r;
        }

        std::experimental::filesystem::path filePath(fileName);
        cv::String fileFormat = opts ? opts->GetString("FileFormat") : "text";
        std::ofstream ofs(filePath, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
        if (ofs.fail())
        {
            errMsg = "open/create file error";
            return MLR_FILESYSTEM_EXCEPTION;
        }
        else
        {
            boost::iostreams::filtering_ostream fout;
            fout.push(boost::iostreams::lzma_compressor());
            fout.push(ofs);
            if (fileFormat == "binary")
            {
                boost::archive::binary_oarchive oa(fout);
                oa << boost::serialization::make_nvp(label, c);
            }
            else if (fileFormat == "text")
            {
                boost::archive::text_oarchive oa(fout);
                oa << boost::serialization::make_nvp(label, c);
            }
            else
            {   // xml
                boost::archive::xml_oarchive oa(fout);
                oa << boost::serialization::make_nvp(label, c);
            }
        }
    }
    catch (const std::exception &e)
    {
        errMsg = e.what();
        return MLR_IO_STREAM_EXCEPTION;
    }

    return MLR_SUCCESS;
}

template <class T>
int LoadFromFile(T &c, const char *label, const cv::String &fileName, const cv::Ptr<Dict> &opts, cv::String &errMsg)
{
    try
    {
        cv::String formatHint;
        std::experimental::filesystem::path filePath(fileName);
        int r = Util::CheckCompressLoadParameters(fileName, opts, formatHint, errMsg);
        if (MLR_SUCCESS != r)
        {
            return r;
        }

        std::ifstream ifs(filePath, std::ifstream::in | std::ifstream::binary);
        if (ifs.fail())
        {
            errMsg = "open file error";
            return MLR_FILESYSTEM_EXCEPTION;
        }
        else
        {
            boost::iostreams::filtering_istream fin;
            fin.push(boost::iostreams::lzma_decompressor());
            fin.push(ifs);
            if (formatHint == "binary")
            {
                boost::archive::binary_iarchive ia(fin);
                ia >> boost::serialization::make_nvp(label, c);
            }
            else if (formatHint == "text")
            {

                boost::archive::text_iarchive ia(fin);
                ia >> boost::serialization::make_nvp(label, c);
            }
            else
            {   // xml
                boost::archive::xml_iarchive ia(fin);
                ia >> boost::serialization::make_nvp(label, c);
            }
        }
    }
    catch (const std::exception &e)
    {
        errMsg = e.what();
        return MLR_IO_STREAM_EXCEPTION;
    }

    return MLR_SUCCESS;
}

}
}

#endif //__OPENCV_MVLAB_UTILITY_HPP__
