#!/usr/bin/perl
use strict;
use warnings;
use matrix;

my @headers;
my @chains = ([]);
while (<>) {
	if (!@headers) {
		@headers = split(",",$_);
		next;
	}
	my ($date, @rates) = split(",",$_);
	push @{$chains[0]}, \@rates;
	foreach my $order (0 .. $#chains) {
		my $chain = @{$chains[$order]};
		last if @{$chain} < @rates;
		my @chunk = @{$chain}[@{$chain} - @rates .. @{$chain}-1];
		my @matrix = &columnConstraints(scalar(@{$chain}));
		foreach (0 .. $#chunk-1) {
			my ($v1, $v2) = ($chunk[$_], $chunk[$_+1]);
			foreach (@{$v2}) {
				push @matrix, [@{$v1},$_];
			}
		}
		
		@rates = @{&matrix::solve(\@matrix)};
		$chains[$order+1] = [] if !defined($chains[$order+1]);
		push @{$chains[$order+1]}, \@rates;
	}
	
}	
		
			
	
sub columnConstraints {
	my $dimension = shift;
	my @rows;
	foreach my $r (0 .. $dimension-1) {
		my @row = ();
		foreach my $c (0 .. $dimension-1) {
			foreach my $y (0 .. $dimension-1) {
				push @row, $y == $r? 1: 0;
			}
		}
		push @row,1;
		push @rows, \@row;
	}
	return @rows;
}

