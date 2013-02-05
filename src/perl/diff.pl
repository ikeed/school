#!/usr/bin/perl
use strict;
use warnings;

my $last = 0;
while (<>) {
	my $this = $_;
	if ($this + $last) {
		printf "%f\n", ($this - $last)/($this + $last);
	}
	$last = $this;
}
 
