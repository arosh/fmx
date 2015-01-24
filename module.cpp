#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <type_traits>
#include <iomanip>

#ifdef PYTHON_MODULE
#include <boost/python.hpp>
#endif

#include "cmdline/cmdline.h"
#include "picojson/picojson.h"
#include "sais/sais.hxx"
#include "wavelet_matrix.h"

using namespace std;
#ifdef PYTHON_MODULE
namespace py = boost::python;
#endif

typedef int64_t index_type;
#define iter(c) decltype((c).size())

vector<uint8_t> bwt(const vector<uint8_t> &S) {
  vector<uint8_t> T(S.size());
  T.shrink_to_fit();
  vector<index_type> tmp(S.size());

  vector<uint8_t> &S2 = const_cast<vector<uint8_t> &>(S);

  saisxx_bwt(S2.begin(), T.begin(), tmp.begin(), (index_type)S.size());
  return T;
}

vector<uint8_t> ibwt(const vector<uint8_t> &T, const index_type pid) {
  vector<uint64_t> C(256, 0);
  for (const auto &c : T) {
    ++C[c];
  }

  uint64_t sum = 0;
  for (iter(C) i = 0; i < C.size(); ++i) {
    uint64_t cur = C[i];
    C[i] = sum;
    sum += cur;
  }

  vector<index_type> phi(T.size());
  for (iter(T) i = 0; i < T.size(); ++i) {
    phi[C[T[i]]] = i;
    ++C[T[i]];
  }

  vector<uint8_t> S(T.size());
  index_type j = pid;
  for (iter(T) i = 0; i < T.size(); ++i) {
    S[i] = T[phi[j]];
    j = phi[j];
  }

  return S;
}

string read_oneline(const string &ifname) {
  ifstream ifs(ifname);

  if (!ifs) {
    perror(ifname.c_str());
    exit(EXIT_FAILURE);
  }

  string s;
  getline(ifs, s);
  return s;
}

void show_string_in_hex(const string &s) {
  for (iter(s) i = 0; i < s.size(); ++i) {
    if (i > 0)
      cerr << ' ';
    unsigned char c = s[i];
    cerr << "0x" << hex << (int)c;
  }
  cerr << endl;
}

bool is_utf8_start_char(unsigned char c) {
  // 2バイト目以降は先頭ビットが0b10である
  if ((c >> 6) == 2)
    return false;
  return true;
}

pair<index_type, index_type> fm_index(const WaveletMatrix &wat,
                                      const string &p) {
  index_type sp = 0;
  index_type ep = wat.length();
  for (iter(p) i = 0; i < p.size(); ++i) {
    unsigned char c = p[p.size() - 1 - i];
    sp = wat.rank_lt(c) + wat.rank(c, sp);
    ep = wat.rank_lt(c) + wat.rank(c, ep);
    if (sp >= ep)
      return make_pair(-1, -1);
  }
  return make_pair(sp, ep);
}

vector<vector<string>> read_datasets(const string &cfname) {
  ifstream ifs(cfname);

  if (!ifs) {
    perror(cfname.c_str());
    exit(EXIT_FAILURE);
  }

  picojson::value json;
  string err = picojson::parse(json, ifs);

  if (!err.empty()) {
    cerr << err << endl;
    exit(EXIT_FAILURE);
  }

  picojson::array &array = json.get<picojson::array>();
  vector<vector<string>> ret;
  for (const picojson::value &it : array) {
    picojson::object book = it.get<picojson::object>();
    string filename = book["filename"].get<string>();
    string title = book["title"].get<string>();
    string author = book["author"].get<string>();
    vector<string> vs = { read_oneline(filename), title, author };
    vs.shrink_to_fit();
    ret.emplace_back(std::move(vs));
  }
  ret.shrink_to_fit();
  return ret;
}

static WaveletMatrix wt_text(8);
static WaveletMatrix wt_freq(9);
static vector<vector<string>> datasets;

vector<string> search_(const string &query, const int num) {
  auto spep = fm_index(wt_text, query);
  vector<pair<WaveletMatrix::Index, int>> res = wt_freq.topk<int>(spep.first, spep.second, num);

  vector<string> ret;
  for (const auto &r : res) {
    // r.second == datasets.size() if datasets[:][0][r.second] == '\n'
    if (r.second == (int)datasets.size())
      continue;
    ostringstream oss;
    oss << r.first << '\t' << datasets[r.second][1] << '\t' << datasets[r.second][2];
    ret.emplace_back(oss.str());
  }
  return ret;
}

#ifdef PYTHON_MODULE
py::list search(const string &query, const int num) {
  py::list ret;
  vector<string> v = search_(query, num);
  for(const string &item : v) {
    ret.append(py::str(item).decode("utf_8"));
  }
  return ret;
}

// queryでtop(num)をsearchしたときにk番目に現れる文書のl番目の出現場所の前後のlen文字を取得する
py::api::object description(
    const string &query, const int num,
    const int k, const int l, const int len) {
  // 文書のIDを取得する
  auto spep = fm_index(wt_text, query);
  vector<pair<WaveletMatrix::Index, int>> res = wt_freq.topk<int>(spep.first, spep.second, num);
  for(int i = 0; i < res.size(); ++i) {
    if(res[i].second == datasets.size()) {
      res.erase(res.begin() + i);
      --i;
    }
  }
  const int d = res[k].second;
  cerr << "d = " << d << endl;

  // 文書の場所を取得する
  // suffix arrayの実体が必要？
  WaveletMatrix::Index p = wt_freq.select(d, l + wt_freq.rank(d, spep.first));
  cerr << "p = " << p << endl;

  vector<WaveletMatrix::Index> V(datasets.size());
  {
    WaveletMatrix::Index s = 0;
    for(iter(datasets) i = 0; i < datasets.size(); ++i) {
      V[i] = s;
      s += (datasets[i][0].size() + 1);
    }
  }
  p -= V[d];
  cerr << "p = " << p << endl;

  const int prepost = (len - query.length()) / 2;

  WaveletMatrix::Index st = p;
  for(int i = 0; i < prepost && st > 0; ++i) {
    --st;
    uint8_t c = datasets[d][0][st];
    if(c == '\n') {
      ++st;
      break;
    }
  }
  cerr << "st = " << st << endl;
  while(is_utf8_start_char(datasets[d][0][st]) == false) ++st;
  cerr << "st = " << st << endl;

  WaveletMatrix::Index en = p + query.length();
  for(int i = 0; i < prepost && st + 1 < datasets[d][0].size(); ++i) {
    ++en;
    uint8_t c = datasets[d][0][en];
    if(c == '\n') {
      break;
    }
  }
  cerr << "en = " << en << endl;
  while(is_utf8_start_char(datasets[d][0][en]) == false) --en;
  cerr << "en = " << en << endl;

  string ret;
  for(WaveletMatrix::Index i = st; i < en; ++i) {
    ret.push_back(wt_text.access<uint8_t>(i));
  }

  show_string_in_hex(ret);

  return py::str(ret).decode("utf_8");
}
#endif

void init(const string &config) {
  datasets = read_datasets(config);

  {
    vector<uint8_t> SV;
    for (iter(datasets) i = 0; i < datasets.size(); ++i) {
      SV.insert(SV.end(), datasets[i][0].begin(), datasets[i][0].end());
      SV.push_back('\n');
    }
    SV.push_back('\0');
    SV.shrink_to_fit();

    vector<index_type> SA(SV.size());
    saisxx(SV.begin(), SA.begin(), (index_type)SV.size());
    SA.shrink_to_fit();

    {
      vector<uint8_t> TV = bwt(SV);
      TV.shrink_to_fit();
      wt_text.init(TV);
    }

    {
      vector<uint64_t> array(SV.size());
      array.shrink_to_fit();
      uint64_t sum = 0;
      for (iter(datasets) i = 0; i < datasets.size(); ++i) {
        for (iter(datasets[i][0]) j = 0; j < datasets[i][0].size(); ++j) {
          array[sum] = i;
          ++sum;
        }
        array[sum] = datasets.size();
        ++sum;
      }
      array[sum] = datasets.size();
      ++sum;

      vector<uint64_t> rev(SV.size());
      rev.shrink_to_fit();
      for (iter(SV) i = 0; i < SV.size(); ++i) {
        rev[i] = array[SA[i]];
      }
      wt_freq.init(rev);
    }
  }
}

#ifdef PYTHON_MODULE
BOOST_PYTHON_MODULE(fmx) {
  boost::python::def("init", &init);
  boost::python::def("search", &::search);
  boost::python::def("description", &description);
}
#endif
