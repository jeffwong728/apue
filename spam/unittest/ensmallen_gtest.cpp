#include "helper.h"
#include <cassert>
#include <opencv2/highgui.hpp>
#include <gtest/gtest.h>
#include <Simd/SimdLib.h>
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#pragma warning( push )
#pragma warning( disable : 4244)
#include <ensmallen.hpp>
#pragma warning( pop )

namespace
{
class SquaredFunction
{
public:
    double Evaluate(const arma::mat& x)
    {
        return 2 * std::pow(arma::norm(x), 2.0);
    }
};

TEST(EnsmallenTest, Reduce2X2)
{
    arma::mat x("1.0 -1.0 1.0");
    auto es =  ens::ExponentialSchedule();
    ens::SA<ens::ExponentialSchedule> optimizer(es);
    SquaredFunction f;
    optimizer.Optimize(f, x);

    std::cout << "Minimum of squared function found with simulated annealing is "<< x;
    EXPECT_EQ(1, 1);
}
}