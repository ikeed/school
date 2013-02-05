#!/usr/bin/perl -W

system "ps -A";
print "my parent's processID is: " . getppid() . "\n";

