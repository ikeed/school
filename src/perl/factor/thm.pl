#!/usr/bin/perl
use strict;
use warnings;

my $max = shift;

my @primes = &getPrimes($max);
foreach my $i (0 .. $#primes) {
	foreach my $j ($i+1 .. $#primes) {
		my ($p, $q) = ($primes[$i], $primes[$j]);
		#($p, $q) = (7, 59); #***CRBKLURGE***
		my $n = $p * $q;
		printf "%d:\t ((%s),(%s)),\n\t%d:\t ((%s),(%s))\n\t%d:\t ((%s),(%s))\n",
			$n, join(", ", &allFactors(&ev1($n, -1))), join(", ", &allFactors(&ev1($n, 1))),
			$p, join(", ", &allFactors(&ev1($p, -1))), join(", ", &allFactors(&ev1($p, 1))),
			$q, join(", ", &allFactors(&ev1($q, -1))), join(", ", &allFactors(&ev1($q, 1))),
	}
}

sub eviscerate {
	my $n = shift;
	my (@terms, %factors);
	foreach (-1, 1) {
		push @terms, &ev1($n,$_);
	}
	@terms = &factor(@terms);
	@factors{@terms} = ();
	return sort{$a<=>$b} keys(%factors);
}

sub ev1 {
	my ($n, $d) = @_;
	my @terms;
	while ($n > 1) {
		$n+=$d;
		$n/=2 until $n%2;
		push @terms, $n;
	}
	return @terms;
}

sub compare {
	my ($pref, $l1, $l2) = @_;
	my @q1 = @{$l1};
	my @q2 = @{$l2};
	my (@missing2, @missing1);
	foreach my $q (@q1) {
		if (!grep $q, @{$l2}) {
			push @missing2, $q;
		}
	}
	foreach my $q (@q2) {
		if (!grep $q, @{$l1}) {
			push @missing1, $q;
		}
	}
	printf "%s\tThese were missing from the first list: (%s)\n",
		$pref, join(", ", @missing1) if @missing1;
	#printf "These were missing from the second list: (%s)\n",
	#	join(", ", @missing2) if @missing2;
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

sub allFactors {
	my @f;
	foreach (@_) {
		push @f, &factor($_);
	}
	my %uniq;
	@uniq{@f} = ();
	return sort{$a<=>$b} (keys(%uniq));
}

sub factor {
	my $n = shift;
	my %factors;
	foreach my $p (@primes) {
		while ($n > 1 and $n % $p == 0) {
			$factors{$p}++;
			$n/=$p;
		}
	}
	return sort{$a<=>$b} keys(%factors);
}

