#include <iostream>
#include <vector>
#include "wat_array/wat_array.hpp"
using namespace std;
#define DEBUG(x) cerr << #x << " = " << x << endl;
int main() {
  vector<uint64_t> array { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 1 };
  for(vector<uint64_t>::size_type i = 0; i < array.size(); ++i) {
    if(i > 0) cout << ' ';
    cout << array[i];
  }
  cout << endl;

  wat_array::WatArray wat;
  wat.Init(array);
  DEBUG(wat.Select(1, 2));
  DEBUG(wat.Rank(1, 4));
}
