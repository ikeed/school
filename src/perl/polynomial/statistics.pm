#!/usr/bin/perl
use strict;
use warnings;
use vector;

package statistics;

sub mean_std {
	my ($x, $x2);
	my ($n, $n2) = (scalar(@_), scalar(@_)**2);
	foreach (@_) {
		$x += $_/$n;
		$x2 += $_*$_/$n2;
	}
	return ($x, ((@_ * $x2 - $x * $x)**0.5));
}

sub correlation {
	my ($set1, $set2) = @_;
	my $n = @{$set1}-1;
	my ($xyt, $xt, $x2t, $yt, $y2t);
	my ($sx, $sy);
	my $R;
	foreach (0 .. $n) {
		my ($x, $y) = ($set1->[$_], $set2->[$_]);
		$xyt += $x * $y /$n;
		$xt += $x/$n;
		$yt += $y/$n;
		$x2t += $x * $x;
		$y2t += $y * $y;
	}
	$R = ($xyt - $xt * $yt)
		/(($x2t - $xt*$xt)**0.5 * ($y2t - $yt * $yt)**0.5);
	$sx = ($x2t/$n - $xt * $xt);
	$sy = ($y2t/$n - $yt * $yt);
	return ($R, [$xt, $sx],[$yt, $sy]);
}


sub sma {
	my ($bracket, @points) = @_;
	my $total;
	my @vals;
	foreach (0 .. $bracket-1) {
		$total += $points[$_];
		$vals[$_] = $total / ($_+1);
	}
	foreach my $i ($bracket .. $#points) {
		$vals[$i] = ($total += ($points[$i] - $points[$i-$bracket]))
			/$bracket;
	}
	return @vals;
}

sub fma {
	my ($bracket, @points) = @_;
	my @fibs = (1,1,2,3);
	my (@vals, @mostRecent);
	my $denom;

	#approximating golden ratio
	while (@fibs < 200) {
		push @fibs, $fibs[-1] + $fibs[-2];
	}
	@fibs = @fibs[$#fibs-$bracket+1 .. $#fibs];

	foreach (0 .. $#points) {
		push @mostRecent, $points[$_];
		while (@mostRecent > $bracket) {
			shift(@mostRecent);
		}
		# dotProduct sgould use the dimenstion of mostRecent.
		$vals[$_] = &vector::dotProduct(\@mostRecent, \@fibs);
		if ($_ <= $#fibs) {
			$denom += $fibs[$_];
		}
		$vals[$_] /= $denom;
	}
	return @vals;
}

sub normalize {
	my ($m, $s) = &mean_std(@_);
	return map {($_ - $m)/$s} @_;
}


1;

