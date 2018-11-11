 /* example.i */
 %module apue
 %{
#include <vector>
#include <tbb/tbb.h>
#include <tbb/memory_pool.h>


struct SFaceKey
{
    uint64_t ids0;
    uint64_t ids1;
    uint64_t ids2;
};

extern void testSerial();
extern void testPipeline();
extern double average(const std::vector<int> &v);
extern double median(const double v[10]);
extern void testFaceKeyList(const std::vector<SFaceKey, tbb::scalable_allocator<SFaceKey>> &fkv);
%}

struct SFaceKey
{
    uint64_t ids0;
    uint64_t ids1;
    uint64_t ids2;
};

%include stl.i

namespace std 
{
    %template(IntVector) vector<int>;
    %template(SFaceKeyVector) std::vector<SFaceKey, tbb::scalable_allocator<SFaceKey>>;
}

extern void testSerial();
extern void testPipeline();
extern void testFaceKeyList(const std::vector<SFaceKey, tbb::scalable_allocator<SFaceKey>> &fkv);
extern double average(const std::vector<int> &v);
extern double median(const double v[10]);