#!/bin/perl

#print $ARGV[0]."\n";

open FH, "<", $ARGV[0];


my $cnt_line = 0;
my $win_size = $ARGV[1];
my $sum = 0;
my $first_element;

my $index = 0;
my @pre = (1 ... $win_size);
#print join("\n", @pre)."\n";
#exit;

while(<FH>) {
	my @data = split(/\s/, $_);
	#print join(",", @data)."\n";
	
	$cnt_line ++;


	if($cnt_line == 1) {
		$first_element = $data[1];
		$sum = $data[1];
	}
	elsif($cnt_line < $win_size) {
		$sum += $data[1];
	}
	elsif($cnt_line == $win_size) {
		$sum += $data[1];
		print "".($sum/$win_size)."\n";
	}
	else {
		$sum = $sum - $pre[$index] + $data[1];
		print "".($sum/$win_size)."\n";
	}

	$pre[$index] = $data[1];
	$index = ($index + 1) % $win_size;
}

