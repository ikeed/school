#!/usr/bin/perl
# Tcp  password generator server (PROTOTYPE)
# Peter Walsh csci 265

use IO::Socket;
#use Data::SimplePassword;

$sock = new IO::Socket::INET (
                              LocalHost => '',
                              LocalPort => '7071',
                              Proto => 'tcp',
			      Listen => 3,
                              Reuse => 1
                              );
die "Could not create socket: $!\n" unless $sock;

#$pgen = Data::SimplePassword->new;



while ($new_sock = $sock->accept()) {
   $line=<$new_sock>;
   my @chars;
   chop $line;
   ($pnum, $ptype, $psiz) = split(":", $line);

   if ($ptype eq "num") { 
      @chars = (0..9);
   } else {
      @chars = (0..9, 'a'..'z', 'A'..'Z');
   }

   for ($i=0; $i<$pnum; $i++) {
	$pass = &make_password($psiz, \@chars);
      print "$pass \n";
      print $new_sock  $pass , "\n"; 
   }
   print "Send CLOSE to client \n";
   print $new_sock  "CLOSE\n"; 
   close($new_sock);
}

close($sock);

sub make_password {
	my ($n, $alph) = @_;
	my @alphabet = @{$alph};
	my $pass;
	foreach (1 .. $n) {
		$pass .= $alphabet[int(rand() * 10000) % @alphabet];
	}
	return $pass;
}

