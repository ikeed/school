#!/usr/bin/perl
use strict;
use warnings;
use Date::Parse;

my (@gaps, %currentRates);
my %newestRates;
my @ticks;
my @gaps;
open OF, ">outfile" or die "$!";

while (<>) {
	chomp;
	my ($date, $pair, $rate) = split(",",$_);
	my $key = &formatDate($date);
	my ($base, $quote) = split("/",$pair);
	@newestRates{($base, $quote)} = (); #just make them exist.
	push @ticks, [$key, $date, $base, $quote, $rate];	
}

@ticks = sort {$a->[0] <=> $b->[0]} @ticks;
my @currencies = sort keys(%newestRates);
my %cantDo;

$newestRates{"USD"} = 1;
my ($lastKey, $lastDate);
my @outputs;
foreach (0 .. $#ticks) {
	my ($key, $date, $base, $quote, $rate) = @{$ticks[$_]};
	if ($key - $lastKey > 60) {
		push @gaps, [$lastDate, $date]
	}
	($lastKey, $lastDate) = ($key, $date);
	if (!defined($newestRates{$base})) {
		$cantDo{$base} = [] if !exists($cantDo{$base});
		push @{$cantDo{$base}}, $ticks[$_];
		next;
	}
	my $q = $quote;
	foreach ($ticks[$_], @{$cantDo{$quote}}) {
		($key, $date, $base, $quote, $rate) = @{$_};
		$newestRates{$quote} = $rate * $newestRates{$quote};
		push @outputs, [$key, join(",",$date, @newestRates{@currencies})];
	}
	delete $cantDo{$q};
}
@outputs = map{$_->[1]} sort{$a->[0] <=> $b->[0]} @outputs;
printf OF "%s\n", join("\n", @outputs);
close OF;


sub formatDate {        
        my ($ss,$mm,$hh,$day,$month,$year,undef) = &strptime(&str2time(shift));
	return join("", $year, $month, $day, $hh, $mm, $ss);
}
	
