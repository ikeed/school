#!/usr/bin/perl
use strict;
use warnings;

my $max = shift;
my @primes = &getPrimes($max);

foreach my $i (0 .. $#primes) {
	foreach my $j ($i+1 .. $#primes) {
		my $factorme = $primes[$i] * $primes[$j];
		my $found = 0;
		foreach my $d (-1, 1) {
			my $n = $factorme;
			while ($n > 1) {
				$n += $d;
				$n/=2 until $n %2;
				my $g;
				if (($g = &gcd($n, $factorme)) > 1) {
					printf "WOO!  $factorme = $g * %d\n", $factorme/$g;
					$found = 1;
					last;
				}
			}
		}
		printf "$factorme is prime?\n" if !$found;
	}
}

sub gcd {
	my ($a, $b) = @_;
	while ($b) {
		($a, $b) = ($b, $a % $b);
	}
	return $a;
}

sub getPrimes {
	my $max = shift;
	my (%comps, @primes);
	foreach my $p (map {2 * $_ + 1} (1 .. int($max/2))) {
		if (!exists($comps{$p})) {
			push @primes, $p;
			@comps{map{$p * $_} ($p .. int($max/$p))} = ();
		}
	}
	return @primes;
}	


