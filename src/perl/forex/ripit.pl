#!/usr/bin/perl
use strict;
use warnings;

my $delim = ',';
my %quotes;
while (<>) {
	my ($bc, $qc, $mid, $UnixSeconds) = &parseLine($_);
	next if ($bc ne "USD");
	$quotes{$qc} = [] if !exists($quotes{$qc});
	my $l = $quotes{$qc};
	push @{$l}, [$mid, $UnixSeconds];
}	
	
