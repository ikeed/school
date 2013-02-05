#! /usr/bin/perl
use strict;
use warnings;

while (<>) {
	if (/\"(.*)\"/) {
		printf "$1\n";
	}
}

