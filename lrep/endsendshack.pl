#!/usr/bin/perl

# This file removes duplicated .ENDS statements from input files.
# Not sure what bug is generating the need to include this hack.

open IN,  "<$ARGV[0]" or die $!;
open OUT, ">$ARGV[0]_hack" or die $!;

$pline = 'asdf-foo';

while( my $line = <IN>) {

    if($pline eq $line) {
        if($line !~ /\.[Ee][Nn][Dd][Ss]/) {
            print OUT $line;
        }
    }
    else {
        print OUT $line;
    }

    if($line =~ /.*[a-zA-Z]/) {
        $pline = $line;
    }
}

close IN;
close OUT;

@rmorig = ("rm", "-f", "$ARGV[0]");
@cphack = ("cp", "-f", "$ARGV[0]_hack", "$ARGV[0]");
@rmhack = ("rm", "-f", "$ARGV[0]_hack");

system(@rmorig);
system(@cphack);
system(@rmhack);
