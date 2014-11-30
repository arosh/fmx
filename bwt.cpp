#include <cstdint>
#include <cctype>

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <type_traits>
#include <iomanip>

#include "sais/sais.hxx"

using namespace std;

typedef int64_t index_type;

#define SHOW 0

void print_suffix_array(const vector<uint8_t> & S) {
  vector<index_type> SA(S.size());
  saisxx(S.begin(), SA.begin(), (index_type)S.size());

  // cout << "original text: ";
  // for(decltype(S.size()) j = 0; j < S.size(); ++j) {
  //   if(S[j] == 0) {
  //     cout << "\\0";
  //   }
  //   else {
  //     cout << (char)S[j];
  //   }
  // }
  // cout << endl;

#if SHOW
  for(const auto & i : SA) {
    cout << i;
    cout << ": ";
    for(decltype(S.size()) j = i; j < S.size(); ++j) {
      if(S[j] == 0) {
        cout << "\\0";
      }
      else {
        cout << (char)S[j];
      }
    }
    cout << endl;
  }
#endif
}

pair<vector<uint8_t>, index_type> print_bwt(const vector<uint8_t> & S) {
  typedef remove_const<
    remove_reference<decltype(S)>::type
    >::type array_type;
  array_type S2(S); // typeof(S.begin()) != typeof(T.begin()) を回避するため
  array_type T;
  T.resize(S.size());
  vector<index_type> tmp(S.size());

  index_type pid = saisxx_bwt(S2.begin(), T.begin(), tmp.begin(), (index_type)S.size());

#if SHOW
  cout << "pid: " << pid << endl;
  cout << "BWT: ";
  for(decltype(T.size()) i = 0; i < T.size(); ++i) {
    if(T[i] == 0) {
      cout << "\\0";
    }
    else {
      cout << (char)T[i];
    }
  }
  cout << endl;
#endif
  return make_pair(T, pid);
}

vector<uint8_t> print_inverse_bwt(const vector<uint8_t> & T, const index_type pid) {
  typedef make_unsigned<
    remove_reference<
      decltype(*T.begin())
    >::type>::type unsigned_value_type;

  const vector<uint64_t>::size_type k = 256;
  vector<uint64_t> C(k, 0);
  for(const auto & c : T) {
    ++C[(unsigned_value_type)c];
  }

  uint64_t sum = 0;
  for(decltype(C.size()) i = 0; i < C.size(); ++i) {
    uint64_t cur = C[i];
    C[i] = sum;
    sum += cur;
  }

  vector<index_type> phi(T.size());
  for(decltype(T.size()) i = 0; i < T.size(); ++i) {
#if SHOW
    cout << "Tb[" << i << "] <=> SA[" << C[T[i]] << "]" << endl;
#endif
    phi[C[T[i]]] = i;
    ++C[T[i]];
  }

  // for(decltype(phi.size()) i = 0; i < phi.size(); ++i) {
  //   cout << "phi[" << i << "] = " << phi[i] << endl;
  // }

  vector<uint8_t> S;
  S.resize(T.size());
  index_type j = pid;
  for(decltype(T.size()) i = 0; i < T.size(); ++i) {
    S[i] = T[phi[j]];
    j = phi[j];
  }
  return S;
}

int main() {
  string S;
  getline(cin, S);
  S.push_back('\0');
  vector<uint8_t> SV(S.begin(), S.end());

  // print_suffix_array(SV);
#if SHOW
  cout << endl;
#endif

  auto ret = print_bwt(SV);
#if SHOW
  cout << endl;
#endif

  vector<uint8_t> SV2 = print_inverse_bwt(ret.first, ret.second);
#if SHOW
  cout << "BWTinverse: ";
  for(decltype(SV2.size()) i = 0; i < SV2.size(); ++i) {
    if(SV2[i] == 0) {
      cout << "\\0";
    }
    else {
      cout << SV2[i];
    }
  }
  cout << endl;
#endif

  cout << (SV == SV2 ? "OK" : "NG") << endl;
}
