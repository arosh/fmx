#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <type_traits>
#include <iomanip>
#include <deque>
#include <iterator>

#ifdef PYTHON_MODULE
#include <boost/python.hpp>
#endif

#ifdef __GNUC__
// https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html#Diagnostic-Pragmas
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <msgpack.hpp>
#include "cmdline/cmdline.h"
#include "sais/sais.hxx"
#include "picojson/picojson.h"

#pragma GCC diagnostic pop
#endif // __GNUC__

#include "wavelet_matrix.h"

using namespace std;
#ifdef PYTHON_MODULE
namespace py = boost::python;
#endif

typedef int64_t index_type;

struct Book {
  string title, author, content;
  Book() { }
  Book(const string & title_, const string & author_, const string & content_) :
    title(title_), author(author_), content(content_) { }

  MSGPACK_DEFINE(title, author, content);
};

static void show_char_in_hex(const unsigned char c) {
    cerr << "0x" << hex << (int)c;
}

static void show_string_in_hex(const string &s) {
  for (size_t i = 0; i < s.size(); ++i) {
    if (i > 0)
      cerr << ' ';
    unsigned char c = s[i];
    show_char_in_hex(c);
  }
  cerr << endl;
}

static bool is_utf8_start_char(unsigned char c) {
  // 2バイト目以降は先頭ビットが0b10である
  if ((c >> 6) == 2)
    return false;
  return true;
}

pair<index_type, index_type> fm_index(const WaveletMatrix &wat,
                                      const string &p) {
  index_type sp = 0;
  index_type ep = wat.length();
  for (size_t i = 0; i < p.size(); ++i) {
    unsigned char c = p[p.size() - 1 - i];
    sp = wat.rank_lt(c) + wat.rank(c, sp);
    ep = wat.rank_lt(c) + wat.rank(c, ep);
    if (sp >= ep)
      return make_pair(-1, -1);
  }
  return make_pair(sp, ep);
}

static WaveletMatrix wt_text(8);
static WaveletMatrix wt_freq(9);
static vector<Book> datasets;
static vector<uint64_t> bound;

vector<string> search_(const string &query, const int num) {
  auto spep = fm_index(wt_text, query);
  vector<pair<WaveletMatrix::Index, int>> res = wt_freq.topk<int>(spep.first, spep.second, num);

  vector<string> ret;
  for (const auto &r : res) {
    // r.second == datasets.size() if datasets[:][0][r.second] == '\n'
    if (r.second == (int)datasets.size())
      continue;
    ostringstream oss;
    oss << r.first << '\t' << datasets[r.second].title << '\t' << datasets[r.second].author;
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
    // r.second == datasets.size() if datasets[:][0][r.second] == '\n'
    if(res[i].second == datasets.size()) {
      res.erase(res.begin() + i);
      --i;
    }
  }
  const int d = res[k].second;

  // 文書を取得する
  const WaveletMatrix::Index p = wt_freq.select(d, l + wt_freq.rank(d, spep.first));

  const int prepost = (len - query.length()) / 2;
  deque<uint8_t> retDeque;

  WaveletMatrix::Index st = p;
  for(int i = 0; i < prepost; ++i) {
    uint8_t bwtChar = wt_text.access<uint8_t>(st);
    if(bwtChar == '\n' || bwtChar == '\0') break;
    retDeque.push_front(bwtChar);
    st = wt_text.rank_lt(bwtChar) + wt_text.rank(bwtChar, st);
  }
  while(is_utf8_start_char(retDeque.front()) == false) {
    retDeque.pop_front();
  }

  WaveletMatrix::Index en = p;
  for(int i = 0; i < prepost + query.length(); ++i) {
    auto ub = upper_bound(begin(bound), end(bound), en);
    --ub;
    uint8_t saChar = ub - begin(bound);

    if(saChar == '\n' || saChar == '\0') break;
    retDeque.push_back(saChar);
    en = wt_text.select(saChar, en - bound[saChar]);
  }

  while(is_utf8_start_char(retDeque.back()) == false) {
    retDeque.pop_back();
  }
  retDeque.pop_back();

  string ret(retDeque.begin(), retDeque.end());
  return py::str(ret).decode("utf_8");
}
#endif

template <class T>
void unpack(T & ret, const char * fname) {
  ifstream in(fname);
  istreambuf_iterator<char> begin(in), end;
  const string buf(begin, end);
  msgpack::unpacked pc;
  msgpack::unpack(&pc, buf.data(), buf.size());
  pc.get().convert(&ret);
}

void init() {
  unpack(datasets, "msg/book.msg");
  for (auto && book : datasets) {
    book.content.clear();
    book.content.shrink_to_fit();
  }

  {
    vector<uint8_t> bwtVec;
    unpack(bwtVec, "msg/bwt.msg");
    wt_text.init(bwtVec);
  }

  {
    vector<uint16_t> freqVec;
    unpack(freqVec, "msg/book_id.msg");
    wt_freq.init(freqVec);
  }

  bound.resize(256);
  for(uint64_t c = 0; c < 256; ++c) {
    bound[c] = wt_text.rank_lt(c);
  }
}

#ifdef PYTHON_MODULE
BOOST_PYTHON_MODULE(fmx) {
  boost::python::def("init", &init);
  boost::python::def("search", &::search);
  boost::python::def("description", &description);
}
#endif
