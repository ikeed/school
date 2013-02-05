#!/usr/bin/perl
use strict;
use warnings;


my @vals;
while (<>) {
	chomp;
	push @vals, $_;
	if (@vals >= 3) {
		my @arr = @vals[$#vals-2 .. $#vals];
		printf "(%s):\t%f\n", join(", ", @arr), &noncolinearity(@arr);
	}
}

sub noncolinearity {
	my ($y1, $y2, $y3) = @_;
	my $slope = ($y3 - $y1)/2; #2 samples apart.  Do I need to take care of non-constant intervals later?
	my $y2exp = $y1 + $slope;
	return ($y2 - $y2exp)/$y2exp;
}


sub ConditionalProb {
	my ($PB_A, $PA, $PB) = @_;
	return $PB_A * $PA / $PB;
}


