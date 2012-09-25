//just output the file that was received on input
//#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <complex>
#include <iostream>
using namespace std;

typedef std::complex<float>             gr_complex;

void set_data(const char *);

#define DATA_TYPE gr_complex
//#define DATA_TYPE float
//#define DATA_TYPE char

DATA_TYPE *d_data; //to store the data stream
int data_length;


int main (int argc, const char *argv[])
{
    if (argc != 2)
    {
        fprintf (stderr, " usage: %s <data input file>\n", argv[0]);
        exit(1);
    }
    set_data(argv[1]);

    return 0;
}

void set_data(const char *filename)
{
    //char *filename = "data.dat";
    FILE *d_fp;

    if ((d_fp = fopen(filename, "rb")) == NULL)
    {
        fprintf(stderr, "data file cannot be opened\n");
        assert(false);
    }

    //get file size
    fseek( d_fp, 0L, SEEK_END );
    long endPos = ftell( d_fp );
    fclose(d_fp);

    d_data = (DATA_TYPE *) malloc(endPos);

    //re-open file
    if ((d_fp = fopen(filename, "rb")) == NULL)
    {
        fprintf(stderr, "data file cannot be opened\n");
        assert(false);
    }

    int count = fread_unlocked(d_data, sizeof(DATA_TYPE), endPos / sizeof(DATA_TYPE), d_fp);
    count = 30000000;
    //printf ("read in %d entries of data file with size %d\n",count,endPos);

    //   int index = 0;
    for (int index = 0; index < count; index++)
    {
        //     d_preamble[index] = conj(d_preamble[index]);
        //     //printf("importing symbol %d with cong real: %f and imag: %f\n",index, creal(d_preamble[index]), cimag(d_preamble[index]));
        //cout << "importing symbol " << index << " with cong " << d_preamble[index] << " abs " << std::abs(d_preamble[index]) /*<< " arg " << std::arg(d_preamble[index]) */ << "\n" ;
        //cout << index << " " << d_data[index] << '\n';
        cout << index << " " << abs(d_data[index]) << " " << arg(d_data[index]) << " " << d_data[index].real() << " " << d_data[index].imag() << '\n';
        /*
        if(d_data[index].imag() < 1e-6) {
        cout << d_data[index].real() << " 0" << endl;
        } else {
            cout << d_data[index].real() << " " << d_data[index].imag() << endl;
        }*/

    }
    data_length = count;
    fclose(d_fp);
    d_fp = 0;
}

