#! /usr/bin/perl

package vector;

use strict;
use warnings;

sub dotProduct {
	my ($a, $b) = @_;
	my $prod = 0;
	if (@{$a} > @{$b}) {
		printf STDERR "vector_dotProduct passed vectors of differing dimensions.  Not cool.\n";
		return undef;
	}
	map {$prod += $a->[$_] * $b->[$_]} (0 .. @{$a}-1);
	return $prod;
}

sub sum {
	my $sum;
	foreach my $v (@_) {
		map {$sum->[$_] += $v->[$_]} (0 .. @{$v}-1);
	}
	return $sum;
}

sub scalarMult {
	my ($r, @vectors) = @_;
	foreach my $v (@vectors) {
		map {$v->[$_] *= $r} @{$v};
	}
}

sub norm {
	my @norms;
	map {my $n = 0; map {$n += $_ * $_} @{$_};push @norms, $n**0.5;} @_;
}	

1;

