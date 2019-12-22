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
    static inline float dist(const cv::Point2f *p0, const cv::Point2f *p1);
    static inline float dist(const cv::Point2f &p0, const cv::Point2f &p1);
    static inline cv::Point2f midPoint(const cv::Point2f *p0, const cv::Point2f *p1);
    static inline cv::Point2f interPoint(const float t, const cv::Point2f *p0, const cv::Point2f *p1);
    static inline cv::Point2f interPoint(const float t, const cv::Point2f &p0, const cv::Point2f &p1);
    static inline bool nearPoint(const cv::Point2f *p0, const cv::Point2f *p1, const float tol);
    static inline bool farPoint(const cv::Point2f *p0, const cv::Point2f *p1, const float tol);
    static inline cv::Point changedToFixed(const cv::Point2f &point);
    static inline int isLeft(const cv::Point &P0, const cv::Point &P1, const cv::Point &P2);
    static inline float isLeft(const cv::Point2f &P0, const cv::Point2f &P1, const cv::Point2f &P2);
    static inline int isTolLeft(const cv::Point2f &P0, const cv::Point2f &P1, const cv::Point2f &P2, const float tol= G_F_TOL);
    static inline int isLeft(const int P0x, const int P0y, const int P1x, const int P1y, const int P2x, const int P2y);
    static inline vcl::Vec8f isLeft(const vcl::Vec8f &P0x, const vcl::Vec8f &P0y, const vcl::Vec8f &P1x, const vcl::Vec8f &P1y, const vcl::Vec8f &P2x, const vcl::Vec8f &P2y);
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

}
}

#endif //__OPENCV_MVLAB_UTILITY_HPP__
