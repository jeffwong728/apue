#include <string>
#include <iostream>
#include <tbb/tbb.h>
#include <tbb/tick_count.h>
#include <tbb/memory_pool.h>
#include <tbb/flow_graph.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <cassert>
#include <random>
#include <cmath>
#include <vector>
#include <algorithm>
#include <memory>
#include <sstream>
#include <utility>
#include <cmath>
#include <map>
#include <unordered_map>
#include <array>
#include <type_traits>
//#pragma pack(show)

double  My_variable = 3.0;

/* Compute factorial of n */
int fact(int n) {
    if (n <= 1)
        return 1;
    else
        return n*fact(n - 1);
}
 