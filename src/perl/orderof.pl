#! /usr/bin/perl -d
use strict;
use warnings;
use bignum;

my ($a, $m) = @ARGV;
printf "$a has order %d (mod $m)\n", &order_of($a, $m);


sub order_of {
	my ($a, $m) = @_;
	my %seen;
	my %stages;
	my $t = 1;
	my $b = $a;
	my @primeFacts;

	for (my $i = 2; $i <= $m; $i++) {
		if (exists($seen{$a})) {
			#loop detected!
			# a has no order mod m
			return undef;
		}
		$t *= $i;
		$a = &mod_exp($a, $i, $m);
		$stages{$t} = $a;
		my @pfac = &factor($i);
		foreach (0 .. $#pfac) {
			$primeFacts[$_]+= $pfac[$_] if (defined($pfac[$_]));
		}
		if ($a == 1) {
			# a^t == 1 (mod m) -> a^t > m
			my $min = int(log($m)/log($b)) + 1;
			# also, for every exponent before this one, 
			my @candidates = &eligibleOrders(\@primeFacts, $min, 
				\%stages);
			
			foreach (@candidates) {
				if (&mod_exp($b, $_, $m) == 1) {
					return $_;
				}
			}
			die "uh-oh!";
		}
	}
}

sub factor {
	my $n = shift;
	my @expvect;
	while ($n % 2 == 0) {
		$n/=2;
		$expvect[2]++;
	}
	my $d = 3;
	while ($n > 1) {
		while ($n % $d == 0) {
			$expvect[$d]++;
			$n/=$d;
		}
		$d+=2;
	}
	return @expvect;
}

sub eligibleOrders {
	my ($factors, $min, $exclude) = @_;
	my @candidates;
	my @expVector = map(0, (1 .. scalar(@{$factors})));
	my $d; 

	do {
	 	$d = &buildFromFactors(@expVector);
		if ($d < $min) {
			#skip it
		}else {
			my $keeper = 1;
			foreach (keys(%{$exclude})) {
				if ($_ % $d == 0) {
					$keeper = 0;
					last;
				}
			}
			if ($keeper) {
				push @candidates, $d;
			}
		}
	}while (&increment(\@expVector, $factors));
	if (@candidates == 0) {
		push @candidates,  $d;
	}
	return @candidates;
}

sub increment {
	my ($vector, $maxvector) = @_;
	foreach (2 .. scalar(@{$maxvector})-1) {
		if ($vector->[$_] == $maxvector->[$_]) {
			$vector->[$_] = 0;
		}elsif ($vector->[$_] < $maxvector->[$_]) {
			$vector->[$_]++;
			return 1;
		}
	}
	#fell off the end.
	return 0;
}			
			
		
sub buildFromFactors {
	my $r = 1;
	my @exponentVector = @_;

	foreach (2 .. $#exponentVector) {
		$r *= $_**$exponentVector[$_] if defined($exponentVector[$_]);
	}
	return $r;
}


sub mod_exp {
	my ($base, $exp, $mod) = @_;
	my $result = 1;
	while ($exp) {
		if ($exp % 2) {
			$result = ($result * $base) % $mod;
		}
		$exp = int($exp/2);
		$base = ($base * $base) % $mod;
	}
	return $result;
}

#TODO: Make this not suck
sub divisors {
	my $a = shift;
	my @divs;
	for (my $i = 1; $i <= $a; $i++) {
		push @divs, $i if ($a % $i == 0);
	}
	return @divs;
}

