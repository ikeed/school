#!/usr/bin/perl
use strict;
use warnings;
use bignum;

while (<>) {
	my $n = $_;
	my ($div1, $div2) = &divisors($n);
	printf "$n = $div1 * $div2\n";
}

sub divisors {
	
	
		
		

			
