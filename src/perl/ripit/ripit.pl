#!/usr/bin/perl
use strict;
use warnings;
use Math::Trig;

my $pi2 = pi * 2;
my ($n, $x);

my $f = sub {(cos($n * exp(-$x) * $pi2) - cos($pi2 * exp($x)))/2};
my $df = sub {exp(-$x) * pi * ($n * sin($pi2 * $n * exp(-$x))+exp($x * 2) * sin($pi2 * exp($x)))};

my $ddf = sub {pi * exp(-$x * 2) * (- $pi2 * $n * $n * cos($pi2 * $n * exp(-$x)) - $n * exp($x) * sin($pi2 * $n * exp(-$x)) + exp(3 * $x) * sin($pi2 * exp($x)) + $pi2 * exp(4 * $x) * cos($pi2 * exp($x)))};


