#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <type_traits>
#include <iomanip>

#include "cmdline/cmdline.h"
#include "picojson/picojson.h"
#include "sais/sais.hxx"
#include "wat_array/wat_array.hpp"

using namespace std;

typedef int64_t index_type;
#define iter(c) decltype((c).size())

pair<vector<uint8_t>, index_type>
bwt(const vector<uint8_t> & S) {
  vector<uint8_t> T(S.size());
  vector<index_type> tmp(S.size());

  vector<uint8_t> & S2 = const_cast<vector<uint8_t> &>(S);

  index_type pid = saisxx_bwt(S2.begin(), T.begin(), tmp.begin(), (index_type)S.size());

  return make_pair(T, pid);
}

vector<uint8_t>
ibwt(const vector<uint8_t> & T, const index_type pid) {
  vector<uint64_t> C(256, 0);
  for(const auto & c : T) {
    ++C[c];
  }

  uint64_t sum = 0;
  for(iter(C) i = 0; i < C.size(); ++i) {
    uint64_t cur = C[i];
    C[i] = sum;
    sum += cur;
  }

  vector<index_type> phi(T.size());
  for(iter(T) i = 0; i < T.size(); ++i) {
    phi[C[T[i]]] = i;
    ++C[T[i]];
  }

  vector<uint8_t> S(T.size());
  index_type j = pid;
  for(iter(T) i = 0; i < T.size(); ++i) {
    S[i] = T[phi[j]];
    j = phi[j];
  }

  return S;
}

string
read_oneline(const string & ifname) {
  ifstream ifs(ifname, ios::in);

  if(!ifs) {
    perror(ifname.c_str());
    exit(EXIT_FAILURE);
  }

  string s;
  getline(ifs, s);
  return s;
}

void show_string_in_hex(const string & s) {
  for(iter(s) i = 0; i < s.size(); ++i) {
    if(i > 0) cout << ' ';
    unsigned char c = s[i];
    cerr << "0x" << hex << (int)c;
  }
  cerr << endl;
}

bool is_utf8_start_char(unsigned char c) {
  // 2バイト目以降は先頭ビットが10である
  if((c >> 6) == 2) return false;
  return true;
}

pair<index_type, index_type>
fm_index(const wat_array::WatArray & wat, const string & p) {
  index_type sp = 0;
  index_type ep = wat.length();
  for(iter(p) i = 0; i < p.size(); ++i) {
    unsigned char c = p[p.size() - 1 - i];
    sp = wat.RankLessThan(c, wat.length()) + wat.Rank(c, sp);
    ep = wat.RankLessThan(c, wat.length()) + wat.Rank(c, ep);
    if(sp >= ep) return make_pair(-1, -1);
  }
  return make_pair(sp, ep);
}

int
main(int argc, char * argv[]) {
  cmdline::parser parser;
  parser.add<string>("text", 't', "text file name", /* need = */ true);
  parser.add<string>("query", 'q', "query string", /* need = */ false);
  parser.add<string>("meta", 'm', "meta data", /* need = */ true);
  parser.add("help", 'h', "show help");

  if(!parser.parse(argc, argv) || parser.exist("help")) {
    cout << parser.error_full() << parser.usage() << endl;
    exit(EXIT_FAILURE);
  }

  string q;
  if(parser.exist("query")) {
    q = parser.get<string>("query");
  }
  else {
    cout << "query> ";
    cin >> q;
  }

  string ifname = parser.get<string>("text");
  string S = read_oneline(ifname);
  S.push_back('\0');
  vector<uint8_t> SV(S.begin(), S.end());

  auto ret = bwt(SV);
  vector<uint64_t> array(ret.first.begin(), ret.first.end());
  wat_array::WatArray wat;
  wat.Init(array);

  vector<index_type> SA(SV.size());
  saisxx(SV.begin(), SA.begin(), (index_type)SV.size());

  auto spep = fm_index(wat, q);
  for(index_type i = spep.first; i < spep.second; ++i) {
    index_type st = max<index_type>(0, SA[i] - 40);
    index_type en = min<index_type>(S.size(), SA[i] + 40);
    while(is_utf8_start_char(S[st]) == false) ++st;
    if(en < S.size()) {
      while(is_utf8_start_char(S[en]) == false) --en;
    }
    cout << setw(6) << setfill(' ') << SA[i] << ": " << S.substr(st, en - st) << endl;
  }
  cout << spep.second - spep.first << "件ヒットしました" << endl;
}
