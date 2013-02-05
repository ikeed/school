#! /usr/bin/perl
use strict;
use warnings;

while (<>) {
	my $s = $_;
	my $t = "";
	foreach my $i (0 .. length($s)-1) {
		$t .= sprintf "%d  ", ord(substr($s, $i, 1));
	}
	printf "%s\n%s\n", $s, $t;
}

