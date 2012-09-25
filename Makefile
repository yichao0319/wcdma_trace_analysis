# Makefile

CPP = g++
CPPFLAGS = -g -Wall
LDFLAGS = -lm


all: autocorr autocorr2 output

autocorr: autocorrelate.o wcdma_codes.o
	$(CPP) $(CPPFLAGS) $(LDFLAGS) autocorrelate.o wcdma_codes.o -o autocorr

autocorr.o: autocorrelate.cc
	$(CPP) -c autocorrelate.cc

wcdma_codes.o: wcdma_codes.cc
	$(CPP) -c wcdma_codes.cc


autocorr2: autocorrelate2.cc
	$(CPP) $(CPPFLAGS) $(LDFLAGS) autocorrelate2.cc -o autocorr2


output: output_ycc.cc
	$(CPP) $(CPPFLAGS) $(LDFLAGS) output_ycc.cc -o output


clean:
	rm -f *.o autocorr autocorr2 output