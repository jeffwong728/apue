#ifdef HAVE_OPENCV_MVLAB
typedef std::vector<cv::Ptr<cv::mvlab::Region>> vector_Ptr_Region;
typedef std::vector<cv::Ptr<cv::mvlab::Contour>> vector_Ptr_Contour;
typedef std::vector<cv::Point2d> vector_Point2d;

template<> struct pyopencvVecConverter<Ptr<cv::mvlab::Region>>
{
    static bool to(PyObject* obj, std::vector<Ptr<cv::mvlab::Region>>& value, const ArgInfo info)
    {
        return pyopencv_to_generic_vec(obj, value, info);
    }

    static PyObject* from(const std::vector<Ptr<cv::mvlab::Region>>& value)
    {
        return pyopencv_from_generic_vec(value);
    }
};

template<> struct pyopencvVecConverter<cv::Ptr<cv::mvlab::Contour>>
{
    static bool to(PyObject* obj, std::vector<cv::Ptr<cv::mvlab::Contour>>& value, const ArgInfo info)
    {
        return pyopencv_to_generic_vec(obj, value, info);
    }

    static PyObject* from(const std::vector<cv::Ptr<cv::mvlab::Contour>>& value)
    {
        return pyopencv_from_generic_vec(value);
    }
};

template<>
bool pyopencv_to(PyObject* obj, Rect2f& r, const char* name)
{
    CV_UNUSED(name);
    if(!obj || obj == Py_None)
        return true;
    return PyArg_ParseTuple(obj, "ffff", &r.x, &r.y, &r.width, &r.height) > 0;
}

template<>
PyObject* pyopencv_from(const Rect2f& r)
{
    return Py_BuildValue("(ffff)", r.x, r.y, r.width, r.height);
}

#endif
