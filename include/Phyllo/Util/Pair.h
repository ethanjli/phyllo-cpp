#pragma once

// Standard libraries

// Third-party libraries
#include <etl/utility.h> // This indirectly includes ETL_PAIR

// Phyllo


namespace Phyllo { namespace Util {

template<typename First, typename Second>
using Pair = etl::pair<First, Second>;


template<typename First, typename Second>
Pair<First, Second> makePair(First first, Second second) {
  return ETL_MAKE_PAIR(first, second);
}

} }