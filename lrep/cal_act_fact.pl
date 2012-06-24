#! /usr/bin/perl -w


#########################################################################
# cal_act_fact.pl
#########################################################################

# Written 6/20/2003 by Dinesh Patil 
# modified 4/11/2005 by Dinesh Patil


# this script calculates the activity factors from the log info (.out file) 
# from IRSIM. It give the act. factors of only the nets mentioned there. Also 
# the activity factor is defined as the probability of posedges of a signal.
# The energy CV2 is consumed from the supply whenever the signal goes high.
# Hence only this transition is accounted for. Thus for nets with 
# equal rise and fall or stay probablity every cycle, the activity factor is 0.25.
# The output file is given as an argument.
#   The script also calculates the aggregate duty cycle of the signals. This is 
# important for considering the leakage energy of the driving gate when the 
# output turns out to have a very low activity factor. For correct leakage energy
# estimation, one must find the probability of every logic state in the gate..This 
# is very tedious. We assume equal probability in all cases except for special really
# low act. fact. gates which we need to find. 
#   The output is printed in a file given as argument.
#
# 
#########################################################################
# Load Libraries
#########################################################################
use File::Basename;
use Getopt::Long;

($prog) = fileparse($0);
# Check for proper number of arguments and extract args
if ($#ARGV != 4) { # $#ARGV is the number of command line arguments minus 1
    print STDERR "Usage: $prog logFile simFile act_fact_file duty_cycle_file number_of_iterations \n"; # $0 is the script name
    print STDERR "EXAMPLE:  $prog adder32_test.out adder_32_opt.sp.sim adder32_test.power adder32_test.duty 1000 \n";
    exit;
}
#print "Calculating the activity factors.\n";
$infile = shift(@ARGV);
$simfile = shift(@ARGV);
$powfile = shift(@ARGV);
$dutyfile = shift(@ARGV);
$total = shift(@ARGV);
#print "the number of iterations are : $total \n";
open (IN,"<$infile") ||  die  ("Can't open $infile : $!\n");
$iter = 0;
#%act_fact = {};
while(<IN>)
{
		 if(/exit/)
		 {
#					print "exit\n";
					next;
		 }
		 if(/time = /)
		 {
					$iter++;
#print "$iter is the time\n";
					next;
		 }
		 if(! (/=/))
		 {
					next;
		 }
		 @tokens = split / /, $_;
		 if($iter == 0)
		 {
		 		for($i = 1; $i < $#tokens; $i++)
		 		{
						# initialization!
						@sub_tokens = split /=/, $tokens[$i];
						$act_fact{$sub_tokens[0]}{"state"} = $sub_tokens[1] + 0;
						$act_fact{$sub_tokens[0]}{"tran"} = 0;
						$act_fact{$sub_tokens[0]}{"ones"} = 0;
						$act_fact{$sub_tokens[0]}{"zeros"} = 0;
						if($sub_tokens[1] == 1)
						{
								 $act_fact{$sub_tokens[0]}{"ones"} = 1;
						}
						else
						{
								 $act_fact{$sub_tokens[0]}{"zeros"} = 1;
						}
		 		}
		 }
		 else
		 {
				for($i = 1; $i < $#tokens; $i++)
		 		{
						# initialization!
						@sub_tokens = split /=/, $tokens[$i];
						if($sub_tokens[1] == 1)
						{
								 $act_fact{$sub_tokens[0]}{"ones"}++;
						}
						else
						{
								 $act_fact{$sub_tokens[0]}{"zeros"}++;
						}
						if($sub_tokens[1] == 1 && $act_fact{$sub_tokens[0]}{"state"} == 0)
						{		 
								 $act_fact{$sub_tokens[0]}{"state"} = $sub_tokens[1];
								 $act_fact{$sub_tokens[0]}{"tran"}++;
						}
						if($sub_tokens[1] == 0 && $act_fact{$sub_tokens[0]}{"state"}== 1)
						{
								 $act_fact{$sub_tokens[0]}{"state"} = $sub_tokens[1];
						}
#						print "$sub_tokens[0]  =$sub_tokens[1]  \n";
		 		}
		 }
}
close(IN);
open (SIM,"<$simfile") ||  die  ("Can't open $simfile : $!\n");
while(<SIM>)
{
		 if(/^=/)
		 {
					s/^=\s*//;
					@tokens = split /\s+/, $_;
					for($i=0;$i<=$#tokens;$i++)
					{
							 if(exists $act_fact{$tokens[$i]})
							 {
										$numTran = $act_fact{$tokens[$i]}{"tran"};
										$numOnes = $act_fact{$tokens[$i]}{"ones"};
										$numZeros = $act_fact{$tokens[$i]}{"zeros"};
										$boolState = $act_fact{$tokens[$i]}{"state"};
							 }
					}
					for($i=0;$i<=$#tokens;$i++)
					{
							 $act_fact{$tokens[$i]}{"tran"} = $numTran;
               $act_fact{$tokens[$i]}{"ones"} = $numOnes;
				       $act_fact{$tokens[$i]}{"zeros"} = $numZeros;
							 $act_fact{$tokens[$i]}{"state"} = $boolState;
					}
		 }
}
close(SIM);

open (POW,">$powfile") ||  die  ("Can't open $powfile : $!\n");
open (DUT,">$dutyfile") ||  die  ("Can't open $dutyfile : $!\n");
print POW "* #of time stamps: $iter\n";
print DUT "* #of time stamps: $iter\n";
print POW ".POWER\n";
print DUT ".DUTY\n";
for $key (%act_fact)
{
		 if(exists $act_fact{$key}{"tran"})
		 {
		 			$sw_fact = $act_fact{$key}{"tran"}/$total;
					$Rswfact = sprintf("%.5f", $sw_fact);
		 			$duty_fact = $act_fact{$key}{"ones"}/($act_fact{$key}{"ones"} + $act_fact{$key}{"zeros"});
					$RdutyFact = sprintf("%.5f", $duty_fact);
		 			print POW "$key : $Rswfact ;\n";
		 			print DUT "$key : $RdutyFact ;\n";
		 }
}
print POW ".ENDS\n";
print DUT ".ENDS\n";
close(POW);
close(DUT);

