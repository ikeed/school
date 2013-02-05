#!/usr/bin/perl 


use strict;
use warnings;
use vector;
use bignum (p => -350);

package matrix;

sub inverse {
	my $A = &copy(shift);
	my $B;
	my $dimension = &rowCount($A);

	if (!&isInvertible($A)) {
		printf STDERR "inverse asked to process a non-invertible matrix.  "
			 . "what gives?\n";
		&matrix::dump($A);
		return undef;
	}
	$A = &augment($A, &identity($dimension));
	$A = &gaussJordan($A);
	$B = &unaugment($A, $dimension);
	return $B;
}

sub transpose {
	my $M = shift;
	my $MT;
	my ($rc, $cc) = (&rowCount($M), &colCount($M));
	foreach my $x (0 .. $rc-1) {
		foreach my $y (0 .. $cc-1) {
			$MT->[$y]->[$x] = $M->[$x]->[$y];
		}
	}
	return $MT;
}

sub mult {
	my ($A, $B) = @_;
	my $P;
	my ($rc, $cc) = (&rowCount($A), &colCount($A));

	$B = &transpose($B);
	foreach my $x (0 .. $rc -1) {
		foreach my $y (0 .. $cc -1) {
			$P->[$x]->[$y] = &vector::dotProduct($A->[$x],$B->[$y]);
		}
	}
	return $P;
}	

sub copy {
	my $M = shift;
	my $N;
	foreach (@{$M}) {
		push @{$N}, [@{$_}];
	}
	return $N;
}

sub dump {
	my $M = shift;
	foreach my $row (@{$M}) {
		printf "%s\n", join("\t", map(sprintf ("%s", $_), (@{$row})));
	}
} 

sub identity {
	my $dimension = shift;
	my (@I, $row);
	foreach my $r (0 .. $dimension-1) {
		$row = [map($_ == $r? 1 : 0, (0 .. $dimension-1))];
		push @I, $row;
	}
	return \@I;
}

sub gaussJordan {
	my $A = shift;
	my $rows = &rowCount($A);
	my $leadingVal;
	my ($srcrow, $dstrow, $col);

	$A = &gaussianElimination($A);  #should now be in REF
	foreach $srcrow (1 .. $rows-1) {
		return $A if (!defined($col = &colSeekNonZero($A->[$srcrow], $srcrow, $rows-1)));
		$dstrow = 0;
		while (defined($dstrow = &rowSeekNonZero($A, $dstrow, $srcrow-1, $col))) {
			$leadingVal = $A->[$dstrow]->[$col];
			$A->[$dstrow] = &rowOpAdd($A->[$dstrow], $A->[$srcrow], -$leadingVal);
		}
	}
	return $A;
}



sub colSeekNonZero {
	my ($row, $startCol, $endCol) = @_;
	foreach ($startCol .. $endCol) {
		return $_ if ($row->[$_]);
	}
	return undef;
}

sub rowSeekNonZero {
	my ($A, $startRow, $endRow, $col) = @_;
	foreach ($startRow .. $endRow) {
		return $_ if ($A->[$_]->[$col]);
	}
	return undef;
}

sub rowOpScalarMult {
	my ($row, $scalar) = @_;
	foreach (0 .. scalar(@{$row})-1) {
		$row->[$_] *= $scalar;
	}
	return $row;
}	

sub rowOpAdd {
	my ($rowDst, $rowSrc, $scalar) = @_;
	foreach (0 .. scalar(@{$rowDst})-1) {
		$rowDst->[$_] += ($scalar * $rowSrc->[$_]);
	}
	return $rowDst;
}

sub gaussianElimination {
	my $A = shift;
	my $rows = &rowCount($A);
	my ($srcrow, $dstrow, $col);
	my $lead;	

	foreach $col (0 .. $rows -1) {
		next if (!defined($srcrow = &rowSeekNonZero($A, $col, $rows-1, $col)));
		($A->[$col], $A->[$srcrow]) = ($A->[$srcrow], $A->[$col]) unless $srcrow == $col;
		$lead = $A->[$col]->[$col];
		$A->[$col] = &rowOpScalarMult($A->[$col], 1/$lead) unless $lead == 1;
		$A->[$col]->[$col] = 1;
		while ($dstrow = &rowSeekNonZero($A, $col+1, $rows-1, $col)) {
			$lead = $A->[$dstrow]->[$col];
			$A->[$dstrow] = &rowOpAdd($A->[$dstrow], $A->[$col], -$lead);
			$A->[$dstrow]->[$col] = 0;
		} 
	}
	return $A;
}
	

sub augment {
	my ($A, $B) = @_;
	my $rows = &rowCount($A);
	if (&rowCount($B) != $rows) {
		printf STDERR "matrix::augment passed matrices differing in the first dimension.\n";
		return undef;
	}
	foreach (0 .. $rows-1) {
		$A->[$_] = [@{$A->[$_]}, @{$B->[$_]}];
	}
	return $A;
}

sub unaugment {
	my ($A, $ncols) = @_;
	my $B;
	my $rows = &rowCount($A);
	
	$ncols = $rows if !$ncols;
	foreach my $row (0 .. $rows-1) {
		my @lb = ();
		foreach (0 .. $ncols -1) {
			unshift @lb, pop(@{$A->[$row]});
		}
		push @{$B}, \@lb;
	}
	return ($B, $A);
}			

sub rowCount {
	my $matrix = shift;
	return @{$matrix};
}
sub colCount {
	my $matrix = shift;
	return @{$matrix->[0]};
}
sub isSquare {
	my $matrix = shift;
	return &rowCount($matrix) == &colCount($matrix);
}

sub isInvertible {
	my $matrix = shift;
	return (&isSquare($matrix) 
		and &isREF(&gaussianElimination(&copy($matrix))) != 0);
}

sub isREF {
	my $matrix = shift;
	my $rows = &rowCount($matrix);
	my $val;

	foreach my $col (reverse 0 .. $rows-1) {
		foreach my $row (reverse $col .. $rows-1) {
			if ($val = $matrix->[$row]->[$col] != 0) {
				return 0 if ($row != $col);
				return 0 if ($val != 1);
			}
		}
	}
	return 1;
}

#assumes you have checked that the matrix is square.
sub determinant {
	my $A = shift;
	my $rows = &rowCount($A);
	my ($expandrow, $row) = ($rows-1, 0);
	my ($sign, $cofactor, $det) = (1,undef, 0);
	
	if ($rows == 1) {
		return $A->[0]->[0];
	}elsif ($rows == 2) {
		return $A->[0]->[0] * $A->[1]->[1] - $A->[0]->[1] * $A->[1]->[0];
	}
	foreach my $col (0 .. $expandrow) {
		$sign = 1 - (($col + $row) % 2) * 2;
		$cofactor = undef;
		foreach $row (0 .. $expandrow-1) {
			$cofactor->[$row] = [@{$A->[$row]}[0 .. $col-1, $col+1 .. $expandrow]];
		}
		$det += $A->[$expandrow]->[$col] * $sign * &determinant($cofactor);
	}
	return $det;
}	

sub colmult {
	my ($matrix, $column) = @_;
	my $result;
	map {$result->[$_] = &vector::dotProduct($matrix->[$_], $column)} (0 .. @{$column}-1);
	return $result;
}

sub fitPoints {
	my @points = @_;
	my @matrix;
	my @yvals;
	my @polynomial;
	foreach my $point (@points) {
		my ($x, $y) = @{$point};
		push @yvals, $y;
		push @matrix, [map($x**$_, (0 .. $#points))];
	}
	return &colmult(&inverse(\@matrix), \@yvals);
}

sub solve {
	my ($A, $const) = @_;
	my @result;

	$A = &copy($A);
	$A = &augment($A, $const) if defined($const);
	$A = &gaussJordan($A);

	foreach (0 .. &rowCount($A)-1) {
		push @result, $A->[$_]->[-1];
	}
	return \@result;
}
1;	
			
