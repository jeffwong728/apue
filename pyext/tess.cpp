#include <tbb/tbb.h>
#include <tbb/tick_count.h>
#include <tbb/memory_pool.h>
#include <tbb/flow_graph.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <map>
#include <unordered_map>
#include <array>
#include <cassert>
#include <random>
#include <cmath>
#include <vector>
#include <algorithm>
#include <memory>
#include <sstream>
#include <utility>
#include <string>
#include <iostream>
#include <numeric>

struct FaceKey
{
    uint64_t ids[3];
};

struct FaceKeyEqu
{
    bool operator()(const FaceKey& l, const FaceKey& r) const
    {
        return (l.ids[0] == r.ids[0]) && (l.ids[1] == r.ids[1]) && (l.ids[2] == r.ids[2]);
    }
};

struct FaceKeyHash
{
    auto operator()(const FaceKey& fk) const
    {
        auto key = fk.ids[0] ^ fk.ids[1] ^ fk.ids[2];
        return key;
    }
};

class FacetMaper {
    const FaceKey *const m_allFaces;
public:
    std::unordered_map<FaceKey, int, FaceKeyHash, FaceKeyEqu, tbb::scalable_allocator<std::pair<FaceKey, int>>> m_externalFaces;
    void operator()(const tbb::blocked_range<size_t>& r)
    {
        for (auto i = r.begin(); i != r.end(); ++i)
        {
            auto itInsert = m_externalFaces.insert(std::make_pair(m_allFaces[i], 0));
            if (!itInsert.second)
            {
                m_externalFaces.erase(itInsert.first);
            }
        }
    }

    FacetMaper(FacetMaper& x, tbb::split) : m_allFaces(x.m_allFaces) {}

    void join(const FacetMaper& y)
    {
        for (const auto&f : y.m_externalFaces)
        {
            auto itInsert = m_externalFaces.insert(f);
            if (!itInsert.second)
            {
                m_externalFaces.erase(itInsert.first);
            }
        }
    }

    FacetMaper(const FaceKey *const allFaces) :
        m_allFaces(allFaces)
    {}
};

void testMapParrel(const std::vector<FaceKey> &allFaces)
{
    FacetMaper fm(allFaces.data());
    tbb::parallel_reduce(tbb::blocked_range<size_t>(0, allFaces.size(), 100000), fm);
}

typedef std::vector<FaceKey, tbb::scalable_allocator<FaceKey>> FaceKeyList;
void testMapSerial(const FaceKeyList &allFaces, FaceKeyList &extFaces)
{
    std::unordered_map<FaceKey, int, FaceKeyHash, FaceKeyEqu> externalFaces;
    for (const auto &f : allFaces)
    {
        auto itInsert = externalFaces.insert(std::make_pair(f, 0));
        if (!itInsert.second)
        {
            externalFaces.erase(itInsert.first);
        }
    }

    extFaces.reserve(externalFaces.size());
    for (const auto& item : externalFaces)
    {
        extFaces.push_back(item.first);
    }
}

struct TessUnit
{
    FaceKeyList allFaces;
    FaceKeyList extFaces;
};

class TessInput {
public:
    TessInput(std::vector<TessUnit> *const tessUnits, std::vector<TessUnit>::size_type *const pos)
        : m_tessUnits(tessUnits), m_pos(pos) {}

    TessInput(const TessInput& f)
        : m_tessUnits(f.m_tessUnits), m_pos(f.m_pos) {}

    ~TessInput() {}

    TessUnit* operator()(tbb::flow_control& fc) const;
private:
    std::vector<TessUnit> *const m_tessUnits;
    std::vector<TessUnit>::size_type *const m_pos;
};

void genFaces(FaceKeyList &faces)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(1, 5000000);

    FaceKeyList internalFaces(20000);
    for (auto &f : internalFaces)
    {
        uint32_t ids[3] = { dist(gen), dist(gen), dist(gen) };
        std::sort(std::begin(ids), std::end(ids));
        std::copy(std::begin(ids), std::end(ids), f.ids);
    }

    faces.resize(1000);
    for (auto &f : faces)
    {
        uint32_t ids[3] = { dist(gen), dist(gen), dist(gen) };
        std::sort(std::begin(ids), std::end(ids));
        std::copy(std::begin(ids), std::end(ids), f.ids);
    }

    faces.insert(faces.begin(), internalFaces.cbegin(), internalFaces.cend());
    faces.insert(faces.begin(), internalFaces.cbegin(), internalFaces.cend());
    std::shuffle(faces.begin(), faces.end(), std::mt19937(rd()));
}

TessUnit* TessInput::operator()(tbb::flow_control& fc) const
{
    if (*m_pos != m_tessUnits->size())
    {
        auto &allFaces = (*m_tessUnits)[*m_pos].allFaces;
        genFaces(allFaces);

        *m_pos += 1;
        return &(*m_tessUnits)[*m_pos - 1];
    }
    else
    {
        fc.stop();
        return nullptr;
    }
}

class TessFunc {
public:
    void operator()(TessUnit* input) const;
};

void TessFunc::operator()(TessUnit* input) const
{
    testMapSerial(input->allFaces, input->extFaces);
    input->allFaces.clear();
}

void testPipeline()
{
    std::vector<TessUnit> tessUnits(2500);
    std::vector<TessUnit>::size_type pos = 0;

    tbb::filter_t<void, TessUnit*> f1(tbb::filter::serial_out_of_order, TessInput(&tessUnits, &pos));
    tbb::filter_t<TessUnit*, void> f2(tbb::filter::parallel, TessFunc());
    tbb::filter_t<void, void> f = f1 & f2;

    tbb::tick_count t0 = tbb::tick_count::now();
    tbb::parallel_pipeline(20, f);
    tbb::tick_count t1 = tbb::tick_count::now();
    std::cout << (t1 - t0).seconds() << std::endl;
}

void testSerial()
{
    std::vector<TessUnit> tessUnits(2500);

    tbb::tick_count t0 = tbb::tick_count::now();
    for (auto &tessUnit : tessUnits)
    {
        genFaces(tessUnit.allFaces);
        testMapSerial(tessUnit.allFaces, tessUnit.extFaces);
        tessUnit.allFaces.clear();
    }
    tbb::tick_count t1 = tbb::tick_count::now();
    std::cout << (t1 - t0).seconds() << std::endl;
}

double average(const std::vector<int> &v)
{
    return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
}

double median(const double v[10])
{
    std::vector<double> vv(&v[0], &v[10]);
    std::sort(vv.begin(), vv.end());
    return vv[5];
}

struct SFaceKey
{
    uint64_t ids0;
    uint64_t ids1;
    uint64_t ids2;
};

void testFaceKeyList(const std::vector<SFaceKey, tbb::scalable_allocator<SFaceKey>> &fkv)
{
    for (const auto &fk : fkv)
    {
        std::cout << fk.ids0 << " " << fk.ids1 << " "<<fk.ids2 << std::endl;
    }
}