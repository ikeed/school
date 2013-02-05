#!/usr/bin/perl
use strict;
use warnings;

printf "Enter your phrase to be encoded:\n";
my $phrase = uc(<stdin>);
$phrase =~ s/\W//g;
my @vals = map{ord(substr($phrase,$_,1)) - ord("A")} (0 .. length($phrase)-1);
map {printf "%s\n", &equation($_)} @vals;

sub equation {
	my $x = shift;
	my ($m, $b, $y);
	($m, $b) = (&randomCoef(5), &randomCoef(10));
	$y = $m * $x + $b;
	return sprintf("%d = %dX + %d", $y, $m, $b);
}

sub randomCoef {
	my $m = shift;
	my $r = int(rand() * 100000) % $m + 1;
	if ($m > 5 and int(rand() * 4) % 3 == 0) {
		$r *= -1;
	}
	return $r;
}

