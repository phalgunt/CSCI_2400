//-*-c++-*-
#ifndef _Filter_h_
#define _Filter_h_

using namespace std;

class Filter {

public:
  short int divisor;
  short int dim;
  short int *data;
  Filter(int _dim);
  short int get(int r, int c) const;
  void set(int r, int c, int value);

  short int getDivisor() const;
  void setDivisor(int value);

  short int getSize() const;
  void info() const;

};

  inline short int Filter::get(int r, int c) const
  {
    return data[ r * dim + c ];
  }

  inline void Filter::set(int r, int c, int value)
  {
    data[ r * dim + c ] = value;
  }

  inline short int Filter::getDivisor() const
  {
    return divisor;
  }

  inline void Filter::setDivisor(int value)
  {
    divisor = value;
  }

  inline short int Filter::getSize() const
  {
    return dim;
  }

#endif
