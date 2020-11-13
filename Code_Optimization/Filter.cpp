#include "Filter.h"
#include <iostream>

Filter::Filter(int _dim)
{
  divisor = 1;
  dim = _dim;
  data = new short int[dim * dim];
}



void Filter::info() const
{
  cout << "Filter is.." << endl;
  for (int row = 0; row < dim; row++) {
    for (int col = 0; col < dim; col++) {
      int v = get(row, col);
      cout << v << " ";
    }
    cout << endl;
  }
}
