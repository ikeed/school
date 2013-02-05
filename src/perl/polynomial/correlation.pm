#!/usr/bin/perl
use strict;
use warnings;
use polynomial;
use vector;
use matrix;

package correlation;

sub findCrossCorrealtions {
	my ($epsilon, @plots) = @_;
	my @relations;
	my %vitals;
	map {$vitals{$_} = &calcVitals($_)} @plots;
	

sub calcVitals {
	my @xyplot = @{shift};
	
			
	

1;
