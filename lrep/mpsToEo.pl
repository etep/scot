#! /usr/bin/perl -w


#########################################################################
# mps.pl
#########################################################################

# Written 5/2/2005 by Dinesh Patil 

# This script changes the mps and f file used in the dual geometric optimization
# into an .eo file needed by the exponential optimization. 
#
# In the exponential optimization, there is not equality constraints, so its the 
# users responsibility to see that no such constraints are embedded in the mps,f files 
# We can solve the expOpt in the primal form, hence it is much faster. 
# Check the mosek documentation for details of the formats.
#
# The mps, f and the eo file are given as arguments. The script uses two dummy files 
# "dummyMpsToEoMatrix"  and "dummyMpsToEoCoeff" which are deleted later.

#########################################################################
# Load Libraries
#########################################################################
#None needed so far

# Check for proper number of arguments and extract args
if ($#ARGV != 2) { # $#ARGV is the number of command line arguments minus 1
    print STDERR "Usage: $0 mps_filename f_filename result_file\n"; # $0 is the script name
    print STDERR "EXAMPLE:  add32P1500DDET.mps add32P1500DDET.f add32P1500DDET.eo\n";
    exit;
}
print "Running mpsToEo.pl\n";
$mpsfile = $ARGV[0];
$ffile = $ARGV[1];
$eofile = $ARGV[2];
$dumfileM = "dummyMpsToEoMatrix";
$dumfileC = "dummyMpsToEoCoeff";

open (MPS,"<$mpsfile") ||  die  ("Can't open $mpsfile : $!\n");
open (DUM1,">$dumfileM") ||  die  ("Can't open $dumfileM : $!\n");
$numVar = 0;
$record = 1;
print "Recording the numVar\n";
while($record == 1)
{
		 $_ = <MPS>;
		 if(/\s*E\s+c(\d+)/)
		 {
					$numVar = $1 + 0;
		 }
		 if(/COLUMNS/)
		 {
					$numVar = $numVar - 1;
					$record = 0;
		 }
}
print "Printing the matrix\n";
while(<MPS>)
{
		 if(/\s*x(\d+)\s+c(\d+)\s+([\-\.0-9]+)/)
		 {
					$var1 = $1 - 1;
					$var2 = $2 - 1;
					$value = $3 + 0;
					if($var2 != $numVar)
					{
							 print DUM1 "$var1 $var2 $value\n";
					}
					$numTer = $1 + 0;
		 }
}
close(DUM1);
open (FFF,"<$ffile") ||  die  ("Can't open $ffile : $!\n");
open (DUM2,">$dumfileC") ||  die  ("Can't open $dumfileC : $!\n");
$_ = <FFF>; # omit first line.
$iter = 0;
print "Printing the Coefficients\n";
while($iter < $numTer)
{
		 $iter ++;
		 $_ = <FFF>;
		 print DUM2 $_;
}
$numCon = -1;  # to account for the objective, which is the 0th constr,
print "Printing the constraint connection\n";
while(<FFF>)
{
		 $numCon ++;
		 $terms = $_ + 0;
		 for($i=1;$i<=$terms;$i++)
		 {
					print DUM2 "$numCon\n";
		 }
}
close(DUM2);
open (EO,">$eofile") ||  die  ("Can't open $eofile : $!\n");
print EO "$numCon\n";
print EO "$numVar\n";
print EO "$numTer\n";
close(EO);
`cat $dumfileC >> $eofile`;
`cat $dumfileM >> $eofile`;
`rm -f $dumfileC`;
`rm -f $dumfileM`;
