#!/usr/bin/perl
use strict;
use warnings;

my $study = shift;
$study % 2 or die "$study needs to be an even integer.\n";

my $n = ($study - 1)/2;
my @points;
foreach my $d (1 .. $study) {
	my @vals = grep {($_ % (2 * $d + 1)) == (($study - $d) % (2 * $d +1))} (-$study .. $study);
	push @points, map {[$d, $_]} @vals;
}
printf "%s\n", join("\n", map {sprintf "%d %d", @{$_}} @points);


