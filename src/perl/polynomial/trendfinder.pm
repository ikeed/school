#!/usr/bin/perl

use strict;
use warnings;
use matrix;
use polynomial;

package trendfinder;


sub trendList {
	my $tree = shift;
	my @trends;
	if (&hasLeft($tree)) {
		push @trends, &trendList($tree->{'left'});
	}
	if (exists($tree->{'coefs'}) and defined($tree->{'coefs'})) {
		push @trends, [map{$tree->{$_}} ('a','b','coefs','R')];
	}
	if (&hasRight($tree)) {
		return @trends, &trendList($tree->{'right'});
	}
	return @trends;
}

sub leaves {
	my $tree = shift;
	my @l;

	if (&isLeaf($tree)) {
		return $tree;
	}elsif (&hasLeft($tree)) {
		push @l, &leaves($tree->{'left'});
	}
	if (exists($tree->{'coefs'})) {
		push @l, $tree;
	}
	if (&hasRight($tree)) {
		push @l, &leaves($tree->{'right'});
	}
	return @l;
}

sub internalNodesWithValue {
	my $tree = shift;
	my @inwv;
	if (!&isLeaf($tree) and exists($tree->{'coefs'})) {
		push @inwv, $tree;
	}
	if (&hasLeft($tree)) {
		push @inwv, &internalNodesWithValue($tree->{'left'});
	}
	if (&hasRight($tree)) {
		push @inwv, &internalNodesWithValue($tree->{'right'});
	}
	return @inwv;
}


sub covers {
	my @l = &leaves(shift);
	my (@gaps, @shingles);
	my $last;
	my ($ep1, $ep2);
	foreach (@l) {
		if (defined($last)) {
			($ep1, $ep2) = ($last->{'b'}, $_->{'a'});
			if ($ep1 < $ep2) {
				push @gaps, [$ep1, $ep2];
			}elsif ($ep1 > $ep2) {
				push @shingles, [$ep2, $ep1];
			}
		}
		$last = $_;
	}
	return (\@gaps, \@shingles, [$l[0]->{'a'}, $l[-1]->{'b'}]);
}

			
				
sub criticalPoints {
	my ($tree,$epsilon) = @_;
	my (@cp, $coords1, $coords2, $a, $b);
	my $points = $tree->{'points'};
	my @trends = &trendList($tree);

	if (!defined($epsilon)) {
		$epsilon = 10**-4;
	}

	foreach (0 .. $#trends-1) {
		my ($t1, $t2) = ($trends[$_], $trends[$_+1]);
		($a, $b) = ($t1->[0], $t2->[1]);
		($coords1, $coords2) = ($t1->[2], $t2->[2]);
		my ($b1, $m1, $b2, $m2) = (@{$coords1}, @{$coords2});
		next unless $m1 * $m2 < 0;

		#this is the linear version.  I'm using parabola now
		my $x = ($b2 - $b1)/($m1 - $m2);
		my $y = $m1 * $x + $b1;
		push @cp, [$x, $y];

		#my $poly = &polynomial::approxFitPoints(2, @{$points}[$a .. $b]);
		#my $deriv = &polynomial::differentiate($poly);
		#my @cp2 = &polynomial::solve($deriv, $epsilon, $points->[$a]->[0], $points->[$b]->[1]);
		#push @cp, map{[$_,&polynomial::evaluate($poly, $_)]} @cp2;
	}
	return @cp;
}
			

sub buildTree {
	my ($a,$b, $minConfidence, $points, $parent) = @_;
	my $thisNd = {'points' => $points};
	my $mid;
	my ($r, $coefs) = &linearFit($a, $b, @{$points});

	if (abs($r) >= $minConfidence) {
		&loadNode($thisNd, $r, $coefs, $a, $b, $parent, $points);
		return $thisNd;
	}
	$mid = int(($b+$a)/2);
	$thisNd->{'left'} = &buildTree($a, $mid, $minConfidence, $points, $thisNd);
	$thisNd->{'right'} = &buildTree($mid, $b, $minConfidence, $points, $thisNd);
	if (!&hasLeft($thisNd) or !&hasRight($thisNd)) {
		return undef;
	}
	if (&hasRight($thisNd->{'left'}) and &hasLeft($thisNd->{'right'})) {
		&resolveMiddle($thisNd, $minConfidence, $points);
	}
	return $thisNd;
}


sub isLeaf {
	my $tree = shift;
	return (!&hasLeft($tree) and !&hasRight($tree));
}

sub hasLeft {
	my $tree = shift;
	return (exists($tree->{'left'}) and defined($tree->{'left'}));
}
sub hasRight {
	my $tree = shift;
	return (exists($tree->{'right'}) and defined($tree->{'right'}));
}

sub hasValue {
	return (exists($_->{'coefs'}) and defined($_->{'coefs'}));
}

sub childCount {
	my $tree = shift;
	return (&hasLeft($tree) + &hasRight($tree));
}

sub linearFit {
	my ($a, $b, @points) = @_;
	my ($xave, $x2ave, $xyave, $yave, $y2ave);
	my ($x, $y, $r, $slope, $intercept);
	my $n = ($b-$a+1);
	if ($n < 2) {
		return undef;
	}elsif ($n == 2) {
		$slope = ($points[$b]->[1] - $points[$a]->[1])/($points[$b]->[0] - $points[$a]->[0]);
		$intercept = $points[$b]->[1] - $slope * $points[$b]->[0];
		return (1, [$intercept, $slope]);
	}
	foreach ($a .. $b) {
		my ($x,$y) = @{$points[$_]};
		$xave += $x/$n;
		$yave += $y/$n;
		$x2ave += $x * $x/$n;
		$y2ave += $y * $y/$n;
		$xyave += $x * $y/$n;
	}
	if ($y2ave == $yave * $yave) {
		#special case.  Literally all of the y-values in this interval were zero!
		return (1, [0,0]);
	}
	($slope, $intercept) = ($xyave - $xave * $yave, $x2ave - $xave * $xave);
	$r = $slope/($intercept * ($y2ave - $yave * $yave))**0.5;
	$slope /= $intercept;
	$intercept = $yave - $slope * $xave;
	return ($r, [$intercept, $slope]);
}

sub deleteRightmostLeaf {
	my $root = shift;
	return if (!&hasRight($root));
	if (!&hasRight($root->{'right'})) {
		delete $root->{'right'};
	}else {
		&deleteRightmostLeaf($root->{'root'});
	}
}

sub deleteLeftmostLeaf {
	my $root = shift;
	return if (!&hasLeft($root));
	if (!&hasLeft($root->{'left'})) {
		delete $root->{'left'};
	}else {
		&deleteLeftmostLeaf($root->{'left'});
	}
}

sub propagateDeletion {
	my $condemned = shift;
	my $parent = $condemned->{'parent'};
	
	if (!defined($parent)) {
		#this should never happen.
		die "propagateDeletion passed the root node?";
	}elsif (&hasLeft($parent) and $parent->{'left'} == $condemned) {
		delete $parent->{'left'};
	}elsif (&hasRight($parent) and $parent->{'right'} == $condemned) {
		delete $parent->{'right'};
	}else {
		die "parent pointer points to node which doesn't acknowledge child!";
	}
	if (&hasValue($parent)) {
		#leave it alone.
		return;
	}elsif (&childCount($parent) == 0) {
		#Parent node has run out of reasons to exist.
		&propagateDeletion($parent);
	}elsif (&childCount($parent) == 1) {
		#parent has one child and no value of its own.
		#parent can assume value of child.
		
		#backup parent's parent to maintain integrity of tree
		my $tmp = $parent->{'parent'};
		
		#copy all fields from child node.  
		#note, this will obliterate left, right pointers, causing child to be effectively deleted.
		if (&hasLeft($parent)) {
			$parent = $parent->{'left'};
		}else {
			$parent = $parent->{'right'};
		}
		#restore parent's parent.
		$parent->{'parent'} = $tmp;
	}
}

	
sub resolveMiddle {
	my ($thisNd, $minConfidence, $points) = @_;
	my ($R, $coefs);
	my ($l, $r) = ($thisNd->{'left'}, $thisNd->{'right'});

	$l = $l->{'right'} while &hasRight($l);
	$r = $r->{'left'} while &hasLeft($r);	

	($R, $coefs) = &linearFit($l->{'a'}, $r->{'b'}, @{$points});
	if (abs($R) >= $minConfidence) {	
		&loadNode($thisNd, $R, $coefs, $l->{'a'}, $r->{'b'}, $thisNd->{'parent'}, $points);
		if (&hasLeft($r)) {
			$r = $r->{'left'};
		}else {
			&propagateDeletion($r);
		}
		if (&hasRight($l)) {
			$l = $l->{'right'};
		}else {
			&propagateDeletion($l);
		}
		return 1; #indicate that we made a change in case anyone cates.
	}
	return 0; #didn't make a change.
}			

sub loadNode {
	my ($nd, $R, $coefs, $a, $b, $parent, $points) = @_;
	$nd->{'R'} = $R;
	$nd->{'coefs'} = $coefs;
	$nd->{'a'} = $a;
	$nd->{'b'} = $b;
	$nd->{'parent'} = $parent;
	$nd->{'points'} = $points;
	return $nd;
}

sub dumpTree {
	my $tree = shift;
	my $points = $tree->{'points'};
	if (&hasLeft($tree)) {
		&dumpTree($tree->{'left'});
	}
	my ($a, $b) = ($tree->{'a'}, $tree->{'b'});
	my ($x1, $x2) = ($points->[$a]->[0], $points->[$b]->[0]);
	my $coords = $tree->{'coefs'};

	printf "ix: [%d, %d] x: [%.1f, %.1f]\ty = %.1fx + %.1f\n", $a, $b, $x1, $x2, $coords->[1], $coords->[0];
	if (&hasRight($tree)) {
		&dumpTree($tree->{'right'});
	}
}

sub Rparms {
	my ($poly, @points) = @_;
	my ($xysum, $ysum, $y2sum);
	map {
		my ($x,$y) = ($_->[1], &polynomial::evaluate($poly, $_->[0]));
		$xysum += $x * $y;
		$ysum += $y;
		$y2sum += $y * $y;
	}@points;
	return ($xysum, $ysum, $y2sum);
}

sub R {
	my ($n, $x, $x2, $xy, $y, $y2) = @_;
	return ($n * $xy - $x * $y)/(($n * $x2 - $x**2) * ($n * $y2 - $y**2))**0.5;
}

sub roughLinear {
	my ($a, $b, $minConf, @points) = @_;
	my ($x, $y, $x2sum, $xsum, $xysum, $ysum, $r);
	my ($poly, $off, $n, %usedOffsets, @processedPoints);
	my $N = ($b - $a + 1);
	my $p = int($N**0.5);

	($r,$n) = (0,0);
	if ($N < 2) {
		return undef;
	}elsif ($N == 2) {
		$poly->[1] = ($points[$b]->[1] - $points[$a]->[1])/($points[$b]->[0] - $points[$a]->[0]);
		$poly->[0] = $points[$b]->[1] - $poly->[1] * $points[$b]->[0];
		return ($poly,1,1);
	}
	while ($r < $minConf and $n/$N < 0.5) {
		do {
			$off = int(rand() * $N) % $p;
		}while (exists($usedOffsets{$off}));
		$usedOffsets{$off} = 1;
		foreach my $ix (0 .. $p-1) {
			my $i = $p * $ix + $off + $a;
			($x, $y) = @{$points[$i]};
			$ysum += $y;
			$xysum += $x * $y;
			$x2sum += $x * $x;
			$xsum += $x;
			push @processedPoints, $points[$i];
		}
		if ($n = @processedPoints) {
			$poly->[1] = ($n * $xysum - $xsum * $ysum) / ($n * $x2sum - $xsum * $xsum);
			$poly->[0] = ($ysum - $poly->[1] * $xsum)/$n;
			my ($xys, $ys, $y2s) = &Rparms($poly, @processedPoints);
			$r = &R($n, $xsum, $x2sum, $xys, $ys, $y2s);
			last if $r > $minConf;
		}
	}
	return ($poly, $r, $n/$N);
}
		 

1;
