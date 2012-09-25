#!/bin/perl
use strict;
use Math::Complex;

my @files = (0, 1); # (0); #
my @lowers = (22200000, 22200000, 22200000, 22206100);
my @ranges = (1000, 5000, 50000, 6000); # (1000); #
my $num_snapshot = 10; # 1; #
my $threshold = 7;

for my $file (@files) {
    my $fname;
    if($file == 0) {
        $fname = 'DL_wdata_768.dat';
    }
    elsif($file == 1) {
        $fname = 'DL_wdata_768_shift100k.dat';
    }
    print $fname."\n";


    for my $range_ind (0 ... $#ranges) {
        my $range = $ranges[$range_ind];
         

        for my $snapshot (0 ... $num_snapshot-1) {
            my $lower = $lowers[$range_ind] + $snapshot*$range;
            my $upper = $lowers[$range_ind] + ($snapshot+1)*$range;

            print "  $lower ~ $upper, threshold: $threshold\n";


            system("./autocorr2 $fname $lower $upper $threshold > h");
            my $newname = join("_", ($fname, $range, $lower, $threshold) );
            ## coefficient
            system("gnuplot plot_coeff.plot");
            system("mv coeff.ps coeff.$newname.ps");
            ## IQ plane
            system("gnuplot plot_plane.plot");
            system("mv plane.ps plane.$newname.ps");
            ## magnitude
            system("gnuplot plot_magnitude.plot");
            system("mv magnitude.ps magnitude.$newname.ps");
            ## IQ value
            system("gnuplot plot_IQ_value.plot");
            system("mv iq_value.ps iq_value.$newname.ps");
            
            system("mv h h.$newname.txt");
        }
        
    }
}
