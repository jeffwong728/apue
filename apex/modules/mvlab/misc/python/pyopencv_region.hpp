#ifdef HAVE_OPENCV_MVLAB
typedef std::vector<cv::Ptr<cv::mvlab::Region>> vector_Ptr_Region;
typedef std::vector<cv::Ptr<cv::mvlab::Contour>> vector_Ptr_Contour;
typedef std::vector<cv::Point2d> vector_Point2d;
typedef std::vector<cv::Point3d> vector_Point3d;
typedef std::vector<cv::Point3i> vector_Point3i;
typedef std::vector<cv::Size2f> vector_Size2f;
typedef std::map<cv::String, cv::InputArray> map_String_and_InputArray;

template<> struct pyopencvVecConverter<cv::Ptr<cv::mvlab::Region>>
{
    static bool to(PyObject* obj, std::vector<cv::Ptr<cv::mvlab::Region>>& value, const ArgInfo info)
    {
        return pyopencv_to_generic_vec(obj, value, info);
    }

    static PyObject* from(const std::vector<cv::Ptr<cv::mvlab::Region>>& value)
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
bool pyopencv_to(PyObject *o, std::map<cv::String, cv::InputArray> &p, const ArgInfo& info)
{
    CV_UNUSED(info);
    bool ok = true;
    PyObject* key = NULL;
    PyObject* item = NULL;
    Py_ssize_t pos = 0;

    if (!o || o == Py_None)
        return true;

    if (PyDict_Check(o)) {
        while (PyDict_Next(o, &pos, &key, &item))
        {
            // get key
            std::string k;
            if (!getUnicodeString(key, k))
            {
                ok = false;
                break;
            }

            cv::Mat val;
            if (!pyopencv_to(item, val, ArgInfo(k.c_str(), 0)))
            {
                ok = false;
                break;
            }

            p.emplace(std::make_pair(k, cv::InputArray(val)));
        }
    }

    return ok && !PyErr_Occurred();
}

template<>
bool pyopencv_to(PyObject* obj, Rect2f& r, const ArgInfo& info)
{
    CV_UNUSED(info);
    if(!obj || obj == Py_None)
        return true;
    return PyArg_ParseTuple(obj, "ffff", &r.x, &r.y, &r.width, &r.height) > 0;
}

template<>
PyObject* pyopencv_from(const Rect2f& r)
{
    return Py_BuildValue("(ffff)", r.x, r.y, r.width, r.height);
}

template<>
bool pyopencv_to(PyObject* obj, Size2d& r, const ArgInfo& info)
{
    CV_UNUSED(info);
    if(!obj || obj == Py_None)
        return true;
    return PyArg_ParseTuple(obj, "dd", &r.width, &r.height) > 0;
}

template<>
PyObject* pyopencv_from(const Size2d& r)
{
    return Py_BuildValue("(dd)", r.width, r.height);
}

#endif
