#!/bin/perl
use strict;
use Math::Complex;

my $file = "sync_frame.txt";
my $num_frame = 2;
my $num_slots_per_frame = 15;
my $num_chips_per_slot = 5128;
my $num_chips_per_frame = $num_chips_per_slot * $num_slots_per_frame;


for my $frame_ind (0 ... $num_frame-1) {
    ## entire frame
    # my $lower = $frame_ind * $num_chips_per_frame;
    my $upper = ($frame_ind + 1) * $num_chips_per_frame;
    my $head = $upper-1;
    my $tail = $num_chips_per_frame;
    system("head -$head $file | tail -$tail > h");
    ## coefficient
    # system("gnuplot plot_coeff.plot");
    # system("mv coeff.ps $file.$frame_ind.coeff.ps");
    ## IQ plane
    system("gnuplot plot_plane.plot");
    system("mv plane.ps $file.$frame_ind.plane.ps");
    ## magnitude
    system("gnuplot plot_magnitude.plot");
    system("mv magnitude.ps $file.$frame_ind.magnitude.ps");
    ## IQ value
    system("gnuplot plot_IQ_value.plot");
    system("mv iq_value.ps $file.$frame_ind.iq_value.ps");


    ## individual slots
    for my $slot_ind (0 ... $num_slots_per_frame-1) {
        # my $s_lower = $frame_ind * $num_chips_per_frame + $slot_ind * $num_chips_per_slot;
        my $s_upper = $frame_ind * $num_chips_per_frame + ($slot_ind + 1) * $num_chips_per_slot;
        my $s_head = $s_upper-1;
        my $s_tail = $num_chips_per_slot;
        system("head -$s_head $file | tail -$s_tail > h");
        ## coefficient
        # system("gnuplot plot_coeff.plot");
        # system("mv coeff.ps $file.$frame_ind.$slot_ind.coeff.ps");
        ## IQ plane
        system("gnuplot plot_plane.plot");
        system("mv plane.ps $file.$frame_ind.$slot_ind.plane.ps");
        ## magnitude
        system("gnuplot plot_magnitude.plot");
        system("mv magnitude.ps $file.$frame_ind.$slot_ind.magnitude.ps");
        ## IQ value
        system("gnuplot plot_IQ_value.plot");
        system("mv iq_value.ps $file.$frame_ind.$slot_ind.iq_value.ps");
    }
}


system("cp sync_cpich.txt h ");
system("gnuplot plot_plane_in_channel.plot ");
system("mv plane.ps plane.cpich.ps");
system("mv plane_rotated.ps plane.cpich.rotated.ps");
system("gnuplot plot_IQ_value_in_channel.plot");
system("mv iq_value.ps iq_value.cpich.ps");
system("mv iq_value_rotated.ps iq_value.cpich.rotated.ps");
system("gnuplot plot_magnitude_in_channel.plot");
system("mv magnitude.ps magnitude.cpich.ps");
system("mv magnitude_rotated.ps magnitude.cpich.rotated.ps");
system("gnuplot plot_rotation_in_channel.plot");
system("mv rotation.ps rotation.cpich.ps");

system("cp sync_pccpch.txt h ");
system("gnuplot plot_plane_in_channel.plot ");
system("mv plane.ps plane.pccpch.ps");
system("mv plane_rotated.ps plane.pccpch.rotated.ps");
system("gnuplot plot_IQ_value_in_channel.plot");
system("mv iq_value.ps iq_value.pccpch.ps");
system("mv iq_value_rotated.ps iq_value.pccpch.rotated.ps");
system("gnuplot plot_magnitude_in_channel.plot");
system("mv magnitude.ps magnitude.pccpch.ps");
system("mv magnitude_rotated.ps magnitude.pccpch.rotated.ps");

system("rm h");