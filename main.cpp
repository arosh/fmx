#include <iostream>
#include <algorithm>
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

vector<uint8_t> bwt(const vector<uint8_t> &S) {
  vector<uint8_t> T(S.size());
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
      cout << ' ';
    unsigned char c = s[i];
    cerr << "0x" << hex << (int)c;
  }
  cerr << endl;
}

bool is_utf8_start_char(unsigned char c) {
  // 2バイト目以降は先頭ビットが10である
  if ((c >> 6) == 2)
    return false;
  return true;
}

pair<index_type, index_type> fm_index(const wat_array::WatArray &wat,
                                      const string &p) {
  index_type sp = 0;
  index_type ep = wat.length();
  for (iter(p) i = 0; i < p.size(); ++i) {
    unsigned char c = p[p.size() - 1 - i];
    sp = wat.RankLessThan(c, wat.length()) + wat.Rank(c, sp);
    ep = wat.RankLessThan(c, wat.length()) + wat.Rank(c, ep);
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
    ret.emplace_back(vs);
  }
  return ret;
}

void search(const string &query, const int num,
            const wat_array::WatArray &wt_text,
            const wat_array::WatArray &wt_freq,
            const vector<vector<string>> &datasets) {
  auto spep = fm_index(wt_text, query);
  vector<wat_array::ListResult> res;
  wt_freq.ListModeRange(0, numeric_limits<uint64_t>::max(), spep.first,
                        spep.second, num, res);
  for (const auto &r : res) {
    if (r.c == datasets.size())
      continue;
    cout << r.freq << '\t' << datasets[r.c][1] << '\t' << datasets[r.c][2]
         << endl;
  }
}

int main(int argc, char *argv[]) {
  cmdline::parser parser;
  parser.add<string>("config", 'c', "config json file", /* need = */ true);
  parser.add<string>("query", 'q', "query string", /* need = */ false);
  parser.add<int>("number", 'n', "display first `number` lines",
                  /* need = */ false, /* default = */ 20);
  parser.add("interact", 'i', "launch interactive shell");
  parser.add("help", 'h', "show help");

  if (!parser.parse(argc, argv) || parser.exist("help")) {
    cerr << parser.error_full() << parser.usage() << endl;
    exit(EXIT_FAILURE);
  }

  if (parser.exist("query") == false && parser.exist("interact") == false) {
    cerr << "you should enable --query or --interact" << endl;
    exit(EXIT_FAILURE);
  }

  string cfname = parser.get<string>("config");
  vector<vector<string>> datasets = read_datasets(cfname);

  vector<uint8_t> SV;
  for (iter(datasets) i = 0; i < datasets.size(); ++i) {
    SV.insert(SV.end(), datasets[i][0].begin(), datasets[i][0].end());
    SV.push_back('\n');
  }
  SV.push_back('\0');

  vector<index_type> SA(SV.size());
  saisxx(SV.begin(), SA.begin(), (index_type)SV.size());

  wat_array::WatArray wt_text;
  {
    vector<uint8_t> TV = bwt(SV);
    wt_text.Init(vector<uint64_t>(TV.begin(), TV.end()));
  }

  wat_array::WatArray wt_freq;
  {
    vector<uint64_t> array(SV.size());
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
    for (iter(SV) i = 0; i < SV.size(); ++i) {
      rev[i] = array[SA[i]];
    }
    wt_freq.Init(rev);
  }

  if (parser.exist("query")) {
    string q = parser.get<string>("query");
    search(q, parser.get<int>("number"), wt_text, wt_freq, datasets);
  }

  if (parser.exist("interact")) {
    while (true) {
      cout << "query> ";
      string q;
      if (cin >> q) {
        search(q, parser.get<int>("number"), wt_text, wt_freq, datasets);
      } else {
        break;
      }
    }
  }
}
