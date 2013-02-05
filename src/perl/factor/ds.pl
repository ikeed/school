#!/usr/bin/perl
use strict;
use warnings;

my $max = shift;

my @primes = &getPrimes($max);
my @semiprimes = &semify(\@primes);
foreach my $sp (@semiprimes) {
	my ($a, $b, $c, $d) = &solve($sp);
	printf "$sp:\n(%d, %d, %d, %d)\t", $sp, $a, $b, $c, $d;
	printf "[%d, %d, %d, %d]\n", map {&phi($_)} ($a, $b, $c, $d);
}

sub phi { #the dumb way
	my $n = shift;
	my $c = 0;
	foreach (1 .. $n-1) {
		$c++ if &gcd($n, $_) == 1;
	}
	return $c;
}

sub gcd {
	my ($a, $b) = @_;
	while ($b) {
		($a, $b) = ($b, $a % $b);
	}
	return $a;
}


sub solve {
	my $sp = shift;
	my $m = ($sp+1)/2;
	foreach my $b (1 .. $m) {
		$a = $m - $b;
		my $p = $a * $b;
		foreach my $d (1 .. $p) {
			next if ($p % $d);
			next if ($d + $p/$d + 1 != $m);
			return ($a, $b, $d, $p/$d);
		}
	}
	die "ONOES!";
	return (0,0,0,0);
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

sub semify {
	my $p = shift;
	my @sp;

	shift @{$p}; #fuck 2
	foreach my $i (1 .. scalar(@{$p})-1) {
		push @sp, map {$p->[$i] * $p->[$_]} (0 .. $i-1);
	}
	return @sp;
}

						
