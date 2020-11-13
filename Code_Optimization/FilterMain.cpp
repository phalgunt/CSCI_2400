#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "Filter.h"

using namespace std;

#include "rdtsc.h"

//
// Forward declare the functions
//
Filter * readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input, cs1300bmp *output);

int
main(int argc, char **argv)
{

  if ( argc < 2) {
    fprintf(stderr,"Usage: %s filter inputfile1 inputfile2 .... \n", argv[0]);
  }

  //
  // Convert to C++ strings to simplify manipulation
  //
  string filtername = argv[1];

  //
  // remove any ".filter" in the filtername
  //
  string filterOutputName = filtername;
  string::size_type loc = filterOutputName.find(".filter");
  if (loc != string::npos) {
    //
    // Remove the ".filter" name, which should occur on all the provided filters
    //
    filterOutputName = filtername.substr(0, loc);
  }

  Filter *filter = readFilter(filtername);

  double sum = 0.0;
  int samples = 0;

  for (int inNum = 2; inNum < argc; inNum++) {
    string inputFilename = argv[inNum];
    string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
    struct cs1300bmp *input = new struct cs1300bmp;
    struct cs1300bmp *output = new struct cs1300bmp;
    int ok = cs1300bmp_readfile( (char *) inputFilename.c_str(), input);

    if ( ok ) {
      double sample = applyFilter(filter, input, output);
      sum += sample;
      samples++;
      cs1300bmp_writefile((char *) outputFilename.c_str(), output);
    }
    delete input;
    delete output;
  }
  fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);

}

class Filter *
readFilter(string filename)
{
  ifstream input(filename.c_str());

  if ( ! input.bad() ) {
    int size = 0;
    input >> size;
    Filter *filter = new Filter(size);
    int dv;
    input >> dv;
    filter -> setDivisor(dv);
    for (int i=0; i < size; i++) {
      for (int j=0; j < size; j++) {
	int value;
	input >> value;
	filter -> set(i,j,value);
      }
    }
    return filter;
  } else {
    cerr << "Bad input in readFilter:" << filename << endl;
    exit(-1);
  }
}

//changing class to struct increases efficiency, while still producing correct output
double
applyFilter(class Filter *filter, cs1300bmp *input, cs1300bmp *output)
{

  long long cycStart, cycStop;
  cycStart = rdtscll();

  int w=output -> width = input -> width;
  int h=output -> height = input -> height;

  w-=1;
  h-=1;
  //can call getDivisor only once
  float dv = (1.0/(filter -> getDivisor()));
   int r=0;
   int c=0;

/*
equal to:
#pragma omp parallel for
   for(short i = 0; i < filter->getSize(); i++)
   {
       for(short j = 0; j < filter->getSize(); j++)
       {
           filterArr[i][j] = filter -> get(i, j);
       }
   }
   we know getsize will always be 3
*/
//all possibilities of of the get
  short int fa = filter -> get(0, 0);
  short int fb = filter -> get(0, 1);
  short int fc = filter -> get(0, 2);
  short int fd = filter -> get(1, 0);
  short int fe = filter -> get(1, 1);
  short int ff = filter -> get(1, 2);
  short int fg = filter -> get(2, 0);
  short int fh = filter -> get(2, 1);
  short int fi = filter -> get(2, 2);

#pragma omp parallel for
//multithreading
    for(r = 1; r < h; r++)
    {
      short int rowx = r - 1;
      int rowy = r + 1;

      for(c = 1; c < w; c++)
      {
          short int colx = c - 1;
          short int coly = c + 1;

          // int plane = 0;
          short int red = 0;
          short int green = 0;
          short int blue = 0;

          short int a= (input -> color[0][rowx][colx] * fa);
          short int b= (input -> color[0][rowx][c] * fb);
          short int ca= (input -> color[0][rowx][coly] * fc);
          short int d= (input -> color[0][r][colx] * fd);
          short int e= (input -> color[0][r][c] * fe);
          short int f= (input -> color[0][r][coly] * ff);
          short int g= (input -> color[0][rowy][colx] * fg);
          short int h=(input -> color[0][rowy][c] * fh);
          short int i=(input -> color[0][rowy][coly] * fi);




          red += a + b + ca + d + e + f + g + h + i;

          green += (input -> color[1][rowx][colx] * fa) + (input -> color[1][rowx][c] * fb) + (input -> color[1][rowx][coly] * fc) + (input -> color[1][r][colx] * fd) + (input -> color[1][r][c] * fe) + (input -> color[1][r][coly] * ff) + (input -> color[1][rowy][colx] * fg) + (input -> color[1][rowy][c] * fh) + (input -> color[1][rowy][coly] * fi);

          blue += (input -> color[2][rowx][colx] * fa) + (input -> color[2][rowx][c] * fb) + (input -> color[2][rowx][coly] * fc) + (input -> color[2][r][colx] * fd) + (input -> color[2][r][c] * fe) + (input -> color[2][r][coly] * ff) + (input -> color[2][rowy][colx] * fg) + (input -> color[2][rowy][c] * fh) + (input -> color[2][rowy][coly] * fi);
//does not make much difference storing to variables
      if(dv!=1)
      {
    //mult faster
        red *= dv;
        green *= dv;
        blue *= dv;
      }

    //(expression 1) ? expression 2 : expression 3--->If expression 1 evaluates to true, then expression 2 is evaluated.
        if ( red  < 0 ) {
          red = 0;
        }
        if ( red  > 255 ) {
          red = 255;
        }

        if ( green  < 0 ) {
          green = 0;
        }
        if ( green  > 255 ) {
          green = 255;
        }

        if ( blue  < 0 ) {
          blue = 0;
        }
        if ( blue  > 255 ) {
          blue = 255;
        }


        output -> color[0][r][c] = red;
        output -> color[1][r][c] = green;
        output -> color[2][r][c] = blue;

      }
    }

// double
// applyFilter(class Filter *filter, cs1300bmp *input, cs1300bmp *output)
// {
//
//   long long cycStart, cycStop;
//
//   cycStart = rdtscll();
//
//   output -> width = input -> width;
//   output -> height = input -> height;
//
//
//   for(int col = 1; col < (input -> width) - 1; col = col + 1) {
//     for(int row = 1; row < (input -> height) - 1 ; row = row + 1) {
//       for(int plane = 0; plane < 3; plane++) {
//
// 	output -> color[plane][row][col] = 0;
//
// 	for (int j = 0; j < filter -> getSize(); j++) {
// 	  for (int i = 0; i < filter -> getSize(); i++) {
// 	    output -> color[plane][row][col]
// 	      = output -> color[plane][row][col]
// 	      + (input -> color[plane][row + i - 1][col + j - 1]
// 		 * filter -> get(i, j) );
// 	  }
// 	}
//
// 	output -> color[plane][row][col] =
// 	  output -> color[plane][row][col] / filter -> getdvisor();
//
// 	if ( output -> color[plane][row][col]  < 0 ) {
// 	  output -> color[plane][row][col] = 0;
// 	}
//
// 	if ( output -> color[plane][row][col]  > 255 ) {
// 	  output -> color[plane][row][col] = 255;
// 	}
//       }
//     }
//   }

  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (output -> width * output -> height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
	  diff, diff / (output -> width * output -> height));
  return diffPerPixel;
}
