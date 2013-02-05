#! /usr/bin/perl -d
use strict;
use warnings;

my $epsilon = shift(@ARGV);
my @points;
while (<>) {
	chomp;
	push @points, $_;
}

&findTrends(\@points, 0, $#points, $epsilon);
	
sub newTrend {
	my %nd;
	printf "creating new trend with values (%s)\n", join(", ", @_);
	@nd{('slope', 'intercept', 'MSE', 'a', 'b')} = @_;
	return \%nd;
}

sub findTrends {
	my ($points, $a, $b, $epsilon) = @_;
	my %newNd;

	if ($b <= $a || !@points) {
		return undef;
	}

	my $val = &trendIt(@_);
	printf "in findTrends: [$a, $b]\n";
	if (defined($val)) {
		$newNd{'val'} = $val;
		return \%newNd;
	}
	my $mid = int(($a+$b)/2);
	$newNd{'left'} = &findTrends($points, $a, $mid, $epsilon);
	$newNd{'right'} = &findTrends($points, $mid + 1, $b, $epsilon);
	&resolveMiddle(\%newNd, $points, $epsilon);
	return \%newNd;
}

sub resolveMiddle {
	my ($root, $points, $epsilon) = @_;
	my ($l, $r) = ($root->{'left'}, $root->{'right'});
	my ($lp, $rp, $a, $b);
	my $v;

	if (!defined($l) 
		|| !defined($r) 
		|| !defined($l->{'right'})
		|| !defined($r->{'left'})) {
		return;
	}
	while (defined($l->{'right'})) {
		$lp = $l;
		$l = $l->{'right'};
	}

	while (defined($r->{'left'})) {
		$rp = $r;
		$r = $r->{'left'};
	}
	($a, $b) = ($l->{'a'}, $r->{'b'});
	if (!defined($v = &trendIt($points, $a, $b, $epsilon))) {
		return;
	}
	$root->{'val'} = $v;
	$lp->{'right'} = undef;
	$rp->{'left'} = undef;
}
	
sub trendIt {
	my ($points, $a, $b, $epsilon) = @_;
	my ($xs, $ys, $x2s, $y2s, $xys);
	my ($slope, $intercept, $MSE);
	my $n = ($b - $a + 1);

	if ($n == 3) {
		printf "foo\n";
	}
	if ($n <= 1 or !@{$points}) {
		return undef;
	}
		
	foreach my $x ($a .. $b) {
		my $y = $points->[$x];
		$ys += $y;
		$y2s += $y * $y;
		$xys += $x * $y;
		$xs += $x;
		$x2s += $x * $x;
	}
	my $denom = $n * $x2s - $xs * $xs;
	
	$MSE = ($n * $xys - $xs * $ys);
	if ($MSE) {
		$MSE *= $MSE / ($n * $x2s - $xs * $xs) 
				/ ($n * $y2s - $ys * $ys);
	}
	if ($MSE >= $epsilon) {
		return undef;
	}
	$slope = ($n * $xys - ($xs * $ys))/$denom;
	$intercept = (($x2s * $ys) - ($xs * $xys))/$denom;

	return &newTrend($slope, $intercept, $MSE, $a, $b);
}
	
