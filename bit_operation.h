#pragma once

#include <type_traits>

namespace bit_operation {
using namespace std;
template <class T> bool get_bit(const T c, int i) {
  typedef typename make_unsigned<T>::type unsignedT;
  return (static_cast<unsignedT>(c) >> i) & static_cast<unsignedT>(1);
}
};
