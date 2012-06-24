#!/usr/bin/perl

open(IN,  "<$ARGV[0]")      or die $!;
open(OUT, ">$ARGV[0]_hack") or die $!;

while( my $line = <IN> ) {
	$line =~ s/__/\./g;
	print OUT $line;
}

close(IN);
close(OUT);

@rmorig = ("rm", "-f", "$ARGV[0]");
@cphack = ("cp", "-f", "$ARGV[0]_hack", "$ARGV[0]");
@rmhack = ("rm", "-f", "$ARGV[0]_hack");

system(@rmorig);
system(@cphack);
system(@rmhack);
