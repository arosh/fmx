#include <iostream>
#include <vector>
#include <string>

#include "wat_array/wat_array.hpp"
#include "sais/sais.hxx"

using namespace std;

vector<uint8_t> ibwt(const vector<uint8_t> &bwt) {
  wat_array::WatArray wat;
  vector<uint64_t> array(bwt.begin(), bwt.end());
  wat.Init(array);
  vector<uint8_t> S(bwt.size(), '\0');
  uint64_t ch;
  uint64_t k = wat.Select('\0', 1);
  for(uint64_t i = 0; i < wat.length(); ++i) {
    uint64_t pos;
    wat.QuantileRange(0, wat.length(), k, pos, ch);
    S[i] = ch;
    k = wat.Select(ch, k - wat.RankLessThan(ch, wat.length()) + 1);
  }
  return S;
}

int main() {
  string S;
  getline(cin, S);
  S.push_back('\0');

  vector<uint8_t> mbs(S.begin(), S.end());

  vector<uint8_t> bwt(mbs.size());
  vector<int64_t> tmp(mbs.size());
  saisxx_bwt(mbs.begin(), bwt.begin(), tmp.begin(), (int64_t)mbs.size());

  mbs = ibwt(bwt);
  // mbs.end() - 1 for remove '\0'
  string T(mbs.begin(), mbs.end() - 1);
  cout << T << endl;
}
