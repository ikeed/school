#!/usr/bin/perl
# password generator client (PROTOTYPE)
# Peter Walsh csci 265

use IO::Socket;
$VERSION = 0.6;
$mode = 'num';
$len = 12;
$num = 6;

$sock = new IO::Socket::INET (
                              PeerAddr => '127.0.0.1',
                              PeerPort => '7071',
                              Proto => 'tcp'
                             );
die "Could not create socket: $!\n" unless $sock;

for (my $i = 0; $i <= $#ARGV; $i++) {
	if ($ARGV[$i] =~ /-v/) {
		$myname = `basename $0`;
		chomp $myname;
		printf "$myname version: $VERSION\n";
	}elsif ($ARGV[$i] =~ /-t/) {
		if ($ARGV[$i+1] eq 'num') {
			$mode = 'num';
		}elsif ($ARGV[$i+1] eq 'alnum') {
			$mode = 'alnum';
		}else {
			die &usage();
		}
		$i++;
	}elsif ($ARGV[$i] =~ /-n/) {
		if (int($ARGV[$i+1]) < 1) {
			die &usage();
		}
		$num = $ARGV[$i+1];
	}elsif ($ARGV[$i] =~ /-l/) {
		if (int($ARGV[$i+1]) < 1) {
			die &usage();
		}
		$len = $ARGV[$i+1];
	}
}
	
print $sock "$num:$mode:$len\n";

while (1) {
   $pass = <$sock>;
   chop $pass;
   last if $pass eq "CLOSE";
   print "$pass \n";
}

sub usage {
	$myname = `basename $0`;
	chomp $myname;
	$s = sprintf "Usage: $myname [-n number_of_passwords] [-t type] [-l length] [-v]\n";
	$s .= "\ttype is one of:\n\t\talnum - alpha-numeric characters\n\t\tnum - numeric characters only\n";
	$s .= "\t-v version\n";
	return $s;
}

