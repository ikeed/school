#!/usr/bin/perl
use strict;
use warnings;

my $csvfile = "stock.csv";
open INFILE, "<$csvfile" or die "unable to open $csvfile: $!";
my @values = <INFILE>;
close INFILE;

my $l = shift(@values);
chomp $l;
my @h = split(",", $l);
my %headingIndex;
foreach (0 .. $#h) {
	$headingIndex{$h[$_]} = $_;
}

my @averages = &dailyAverages(\%headingIndex, 5, @values);
my ($max, $min) = (undef, undef);
my ($incr, $decr);
foreach (1 .. $#averages) {
	my ($a, $b) = ($averages[$_ -1], $averages[$_]);
	my $move = ($b - $a)/($b + $a);
	$max = $move if (!defined($max) or $move > $max);
	$min = $move if (!defined($min) or $move < $min);
	$incr++ if ($move > 0);
	$decr++ if ($move < 0);
	printf "%.4f\n", $move;
}
printf "max:\t%.4f\tmin:\t%.4f\tincreasing:\t%.4f\tdecreasing:\t%.4f\n", 
	$max, $min, 100 * $decr/scalar(@averages), 100 * $incr/scalar(@averages);


sub dailyAverages {
	my $heading = shift;
	my $blocksize = shift;
	my @values = @_;
	my @averages;
	my @ret;
	my ($totl, $cnt) = (0,0);

	foreach (@values) {
		chomp;
		my @tokens = split(",", $_);
		next if (@tokens != (keys (%{$heading})));
		push @averages, ($tokens[$heading->{'Open'}] + $tokens[$heading->{'Close'}])/2;
	}
	
	foreach (0 .. $#averages) {
		$totl += $averages[$_];
		$cnt++;
		if ($cnt >= $blocksize or $_ == $#averages) {
			push @ret, $totl/$cnt;
			($totl, $cnt) = (0,0);
		}
	}
	return @ret;
}

