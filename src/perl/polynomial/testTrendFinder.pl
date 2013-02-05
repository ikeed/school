#! /usr/bin/perl
use strict;
use warnings;
use trendfinder;
use polynomial;
use statistics;

my $confidence = 0.96;
my ($minx, $maxx) = (-400, 400);
my $poly = [0, -36, 0, 49/3, 0, -14/5, 0, 1];
my $npoints = 100;
my $step = ($maxx - $minx)/$npoints;
my @points;
for (my $x = $minx; $x <= $maxx; $x+= $step){ 
	push @points, [$x, &polynomial::evaluate($poly,$x)];
}
printf "mean = %.1f, stdev = %.1f\n", &statistics::mean_std(@points);
my $ntrends = 0;
my $target = &polynomial::degree($poly);
my $lastConfidence = 0;
my ($nextLower, $nextHigher) = (0, 1);
my ($tree, @trends);

$tree = &trendfinder::buildTree(0, $#points, $confidence, \@points);
@trends = &trendfinder::trendList($tree);
#turned off for now.
while (0 and $ntrends != $target) {
	$tree = &trendfinder::buildTree(0, $#points, $confidence, @points);
	@trends = &trendfinder::trendList($tree);
	$ntrends = @trends;
	if ($ntrends < $target) {
		if ($confidence - $nextLower < 0.0000001) {
			printf "not working out, sorry.\n";
			last;
		}
		($nextLower, $confidence) = ($confidence, ($nextHigher + $confidence)/2);
		printf "Found %d trends, wanted %d.  Increasing required confidence to $confidence\n",
			$ntrends, $target;
	}elsif ($ntrends > $target) {
		if ($nextHigher - $confidence < 0.0000001) {
			printf "not working out, sorry.\n";
			last;
		}
		($nextHigher, $confidence) = ($confidence, ($nextLower + $confidence)/2);
		printf "Found %d trends, wanted %d.  Slacking required confidence to $confidence\n",
			$ntrends, $target;
	}else {
		printf "hit target! exiting\n";
	}
}
#printf "Found optimal confidence: $confidence\n";

printf "found %d trends..\n", scalar(@trends);
foreach (@trends) {
	my ($a, $b, $poly, $R) = @{$_};
	printf "on [%.1f,%.1f]:\t y = %.1fx + %.1f  (R = $R)\n", $points[$a]->[0],$points[$b]->[0],$poly->[1], $poly->[0];
}

my @crits = &trendfinder::criticalPoints($tree);
printf "found %d critical points..\n", scalar(@crits);
printf "%s\n", join(", ", map {sprintf("(%.1f,%.1f)", $_->[0],$_->[1])} @crits);
my $deriv = &polynomial::differentiate($poly);
@crits = &polynomial::solve($deriv, 0.000001, $minx, $maxx);
@crits = map {[$_, &polynomial::evaluate($poly,$_)]} @crits;
printf "expected: (%s)\n", join(", ", 
	map {sprintf("(%.1f,%.1f)", $_->[0],$_->[1])} @crits);

my ($gaps, $shingles, $interval) = &trendfinder::covers($tree);
if (!@{$gaps} and !@{$shingles}) {
	printf "Tree is a perfect partition of the interval [%d, %d]\n", 
		$points[$interval->[0]]->[0], $points[$interval->[1]]->[0];
}else {
	foreach (@{$gaps}) {
		my ($l,$r) = @{$_};
		print "Gap found: [$l, $r]\n";
	}

	foreach (@{$shingles}) {
		my ($l, $r) = @{$_};
		print "Shingle found: [$l, $r]\n";
	}
}



