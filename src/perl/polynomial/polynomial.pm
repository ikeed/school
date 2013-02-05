#!/usr/bin/perl 

package polynomial;

use strict;
use warnings;


sub new {
	return \@_;
}

sub evaluate {
	my ($poly, $x) = @_;
	my ($y, $xp) = (0,1);
	map {$y += $poly->[$_] * $xp; $xp *= $x} (0 .. @{$poly}-1);
	return $y;
}

sub differentiate {
	my $poly = shift;
	my @deriv;
	foreach (1 .. @{$poly}-1) {
		$deriv[$_ -1] = $poly->[$_] * $_;
	}
	return \@deriv;
}

sub integrate_initials {
	my ($poly, @initial_conditions) = @_;
	my $pair = shift(@initial_conditions);
	$poly = &integrate_indef($poly);
	if (defined($pair)) {
		$poly->[0] = $pair->[1] - &evaluate($poly, $pair->[0]);
	}
	foreach (@initial_conditions) {
		$poly = &integrate_indef($poly);
		$poly->[0] = $pair->[1] - &evaluate($poly, $pair->[0]);
	}
	return $poly;	
}

sub integrate_indef {
	my $poly = shift;
	my @integral;
	my $deg = &degree($poly);
	foreach (0 .. $deg) {
		$integral[$_+1] = $poly->[$_]/($_+1);
	}
	$integral[0] = 0;
	return \@integral;
}

sub integrate_def {
	my ($poly, $a, $b) = @_;
	my $indef = &integrate_indef($poly);
	($a, $b) = ($b, $a) if $b < $a;
	return &evaluate($indef, $b) - &evaluate($indef, $a);
}

#WARNING:  Complex roots not supported.
sub solve {
	my ($poly, $epsilon, $a, $b) = @_;
	my ($fnext, $crit, $zero, $deriv, @roots, @newroots);

	if (defined($a) and defined($b) and abs($b - $a) < $epsilon) {
		return ();
	}
	while (&degree($poly) > 2) {
		$deriv = &differentiate($poly);
		$fnext = sub {
			my ($fx, $dfx) = (&evaluate($poly, $_[0]), &evaluate($deriv, $_[0]));
			return $_[0] if $fx == 0;
			return undef if $dfx == 0;
			return $_[0] - $fx/$dfx;
		};
		my ($zero, $crit) = &newtonMethod($fnext, $a, $b, $epsilon);
		if (defined($crit)) {
			#hit a critical point split the interval and recurse
			return (&solve($poly, $epsilon, $a, $crit), 
				&solve($poly, $epsilon, $crit, $b));
		}elsif (!defined($zero)) {
			#no roots in the interval
			return ();
		}else {
			push @roots, $zero;
			$poly = &linDiv($poly, $zero);
		}
	}
	if (!(@newroots = &quadFormula($poly))) {
		return @roots; #not handling complex roots.
	}
	return (@roots, grep ({$_ < $b and $_ > $a} @newroots));
}

sub newtonMethod {
	my ($fnext, $a, $b, $epsilon) = @_;
	my ($x, $lastx) = (($a+$b)/2, ($a + $b)/2 + 10 * $epsilon);

	while (abs($lastx - $x) >= $epsilon) {
		$lastx = $x;
		if (!defined($x = $fnext->($x))) {
			#critical point.  can't use newton's method here.
			return (undef, $lastx);
		}elsif ((defined($a) and $x <= $a) or
			(defined($b) and $x >= $b)) {
			#::TWEEET:: out of bounds!
			return (undef, undef);
		}
	}
	return ($x, undef);
}


sub linDiv {
	my ($poly, $x) = @_;
	my @quotient;
	my $deg = &degree($poly);
	$quotient[$deg] = 0;
	foreach (reverse 1 .. $deg) {
		$quotient[$_-1] = $x * $quotient[$_] + $poly->[$_];
	}
	pop(@quotient);
	return \@quotient;
}
	
sub degree {
	my $poly = shift;
	return @{$poly} -1;
}

sub quadFormula {
	my $poly = shift;
	my ($c, $b, $a) = map {defined($poly->[$_])? $poly->[$_] : 0} (0 .. 2);
	my $scrim;

	return -$c / $b if !$a; #just a straight line!

	if (($scrim = $b * $b - 4 * $a * $c) < 0) {
		return (); #sorry, complex roots not supported.
	}
	$scrim **= 0.5;
	return ((-$b + $scrim)/(2*$a), (-$b - $scrim)/(2*$a));
}

#each point is [x,y]
sub fitPoints {
	my @points = @_;
	my (@x, @y, $matrix, $inv);
	my $degree = $#points;

	map {push @x, $_->[0]; push @y, $_->[1];} @points;
	foreach my $row (0 .. $#points) {
		foreach my $col (0 .. $degree) {
			$matrix->[$row]->[$col] = $x[$row]**$col;
		}
	}
	if (defined($inv = &matrix_inverse($matrix))) {
		#single matrix
		return &matrix_colmult($inv, \@y);
	}else {
		#non-invertibles not supported at this time
		print STDERR "polynomial::fitPoints: non-singular matrix passed to fitPoints.  Not supported yet.\n"
	}
}

sub approxFitPoints {
	my ($degree, @points) = @_;
	my ($matrix, $x, $y, $xp);
	my @yvect;
	
	foreach (@points) {
		($x, $y) = @{$_};
		$xp = 1;
		foreach my $i (0 .. $degree) {
			$matrix->[0]->[$i] += $xp;
			$yvect[$i]->[0] += $xp * $y;
			$xp *= $x;
		}
		foreach my $i (1 .. $degree) {
			$xp *= $x;
			$matrix->[$i]->[$degree] += $xp;
		}
	}
	
	foreach my $j (1 .. $degree) {
		foreach my $h (0 .. $j-1) {
			($x, $y) = ($h, $j-$h);
			$matrix->[$x]->[$y] = $matrix->[0]->[$j];
			($x, $y) = ($degree - $j + $h, $degree - $h);
			$matrix->[$x]->[$y] 
				= $matrix->[$degree - $j]->[$degree];
		}
	}
	@yvect = @{&matrix::solve($matrix,\@yvect)};
	#$matrix = &matrix::gaussJordan($matrix);
	#foreach my $j (0 .. $degree) {
	#	push @yvect, $matrix->[$j]->[$degree+1];
	#}
	return \@yvect;
}				
	

sub intersection {
	my ($epsilon, @polynomials) = @_;
	my $sum = &vctor::sum(@polynomials);
	pop @{$sum} while abs($sum->[-1]) < $epsilon; 
	return &polynomial::solve($sum, $epsilon);
}

sub product {
	my ($poly1, $poly2);
	return [0] if (!defined($poly1 = shift));
	while (defined($poly2 = shift)) {
		my $poly3 = undef;
		foreach my $n1 (0 .. &degree($poly1)) {
			foreach my $n2 (0 .. &degree($poly2)) {
				$poly3->[$n1+$n2] += $poly1->[$n1] * $poly2->[$n2];
			}
		}
		$poly1 = $poly3;
	}
	return $poly1;
}

		
1;

