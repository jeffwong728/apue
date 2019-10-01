#ifdef HAVE_OPENCV_MVLAB

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

typedef std::vector<Ptr<cv::mvlab::Region>> vector_Ptr_Region;
#endif
