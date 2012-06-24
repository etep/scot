#! /usr/bin/perl -w


#########################################################################
# putSectionInOpt.pl
#########################################################################

# written 10/18/2005 by Dinesh Patil 

# This script takes in the name of the section and looks for that section in the input file 
# specified. 
# It then copies that section to the Optfile (typically _mod.sp). Also if the
# Optfile already has that section, it is overwritten. This helps to put in the activity 
# factors, TM gate info etc in the Optfile, in case we use it again and again to generate 
# these numbers.
#
# If the section appears more than once, it is copied multiple times, but if any section 
# instance is empty, it is not copied. All the occurances
# of a section are deleted from the Optfile. This feature is useful to copy subckts.
#
#########################################################################
# Load Libraries
#########################################################################
use File::Basename;
use Getopt::Long;
($prog) = fileparse($0);
# Check for proper number of arguments and extract args
if ($#ARGV != 2) { # $#ARGV is the number of command line arguments minus 1
    print STDERR "Usage: $prog Optfile  FileToIncludeFrom SectionName \n"; # $0 is the script name
    print STDERR "EXAMPLE:  $prog adder_64mod.sp  foo.sp POWER\n";
    print STDERR "A dummy file called  dummySection_adder_64mod.sp will be used\n";
    exit;
}
$spfile = shift(@ARGV);
$datafile = shift(@ARGV);
$section = shift(@ARGV);
$dumfile = "dummySection_".$spfile;

#delete the section from the Optfile and copy the rest in the dummy file.
open (SP,"<$spfile") ||  die  ("Can't open $spfile : $!\n");
open (DUM,">$dumfile") ||  die  ("Can't open $dumfile : $!\n");
$print = 1;
while(<SP>)
{
		 if(/\s*\.$section/)
		 {
					$print = 0;
		 }
		 if($print != 0)
		 {
					print DUM $_;
		 }
		 if(/\s*\.ENDS/)
		 {
					$print = 1;
		 }
}
#print DUM "\n* added section \n";
close(SP);
#print the section from the datafile.
open (DATA,"<$datafile") ||  die  ("Can't open $datafile : $!\n");
$print = 0;
$foundSection = 0;
while(<DATA>)
{
		 if(/\s*\.$section/)
		 {
					$foundSection ++;
					$print = 1;
					print DUM "\n";
		 }
		 if($print != 0)
		 {
					print DUM $_;
		 }
		 if(/\s*\.ENDS/)
		 {
					$print = 0;
		 }
}
close(DATA);
close(DUM);
if($foundSection == 0)
{
		 print "Section $section not found in $datafile!\n";
		 system("rm $dumfile");
		 exit;
}
`mv $dumfile $spfile`;
