#! /usr/bin/perl -w


#########################################################################
# limVectForDiogen.pl
#########################################################################

# Written 6/20/2003 by Dinesh Patil 

# The length of vectors in IRSIM is fixed to some value. Since the #of 
# primary inputs can be more than this limit, this perl script is used to 
# take that .cmd file and convert it into one containing finite length 
# vectors of atmost specific size. In this file the maximum size is specified 
# as one of the arguments. If there are more than one vector definitions that have 
# lengths > than the set limit, the script acts on all of them. The original name 
# of the vector is changed to vect1,vect2 etc..(appending the 1,2 numbers to it)
#
# The inputs are a prepared .cmd file, the vector length. The input file itself
# is modified and overwritten. This script uses a 
# temporary dummy file called, dummyIRSIMcmdFile.
# 
#########################################################################
# Load Libraries
#########################################################################
#None needed so far

# Check for proper number of arguments and extract args
if ($#ARGV != 1) { # $#ARGV is the number of command line arguments minus 1
    print STDERR "Usage: $0 infile  vect_length\n"; # $0 is the script name
    print STDERR "EXAMPLE:  cal_act_fact.pl test_adder.cmd 64 \n";
    exit;
}
#print "Calculating the activity factors.\n";
$infile = $ARGV[0];
$limit = $ARGV[1];
$dummyfile = "dummyIRSIMcmdFile";
#print "the number of iterations are : $total \n";
open (IN,"<$infile") ||  die  ("Can't open $infile : $!\n");
open (OUT,">$dummyfile") ||  die  ("Can't open $dummyfile : $!\n");
$lineno = 0;
#%act_fact = {};
while(<IN>)
{
		 $lineno ++;
		 if(/^\s*vector/)
		 {
					@tokens = split /\s+/,$_;
					$v = shift(@tokens); #containts the word vector..useless
					$vecName = shift(@tokens);
					$numInputs = $#tokens +1;
					if(exists $vectors{$vecName})
					{
							 print "Check input file, two vectors of same name declared (line $lineno)\n";
							 die;
					}
					else
					{	
#	vecList hash as two keys, one (repeat)  to keep count of how many segments the vector is 
#	broken into and the other (name = its an array) to record the names of these new Vectors.
							 $vecList{$vecName}{repeat} = 0;
							 $vectors{$vecName} = 1;
							 if($#tokens < $limit)
							 {
										print OUT $_;
										next;
							 }
							 else #the vector length is bigger than specified.
							 {
										$nameVec = 0;
										$num = 0;
										while(exists $tokens[$#tokens])
										{
												 $num ++;
												 $NewVecName = $vecName."$num";
												 if(exists $vectors{$NewVecName})
												 {
															next;
												 }
												 $vectors{$NewVecName} = 1;
												 $vecList{$vecName}{repeat} ++;
												 $vecList{$vecName}{name}[$nameVec] = $NewVecName;
												 $nameVec++;
												 $statement = "";
												 if($#tokens < $limit)
												 {
															$length = $#tokens;
												 }
												 else
												 {
															$length = $limit-1;
												 }
												 for($bit=0;$bit<=$length;$bit++)
												 {
															$numInputs --;
															$port = shift(@tokens); 
															$statement = $statement." $port";
												 }
												 print OUT "$v $NewVecName $statement\n";
										}
							 }
					}
					next;
		 }
		 if(/^\s*set/ or /^\s*assert/)
		 {
					@tokens = split /\s+/, $_;
					$s = shift(@tokens);
					$vecName = shift(@tokens);
					if(!(exists $vectors{$vecName}))
					{
							 print "Check input file, vector $vecName used before declaration (line $lineno)\n";
							 die;
					}
					if($vecList{$vecName}{repeat} == 0)
					{
							 print OUT $_;
							 next;
					}
					else
					{
							 @inputs = split(//, shift(@tokens));
							 @dupnames =  @{ $vecList{$vecName}{name}};
							 for($iter = 0;$iter <=$#dupnames ;$iter ++) 
							 {
										$name = $dupnames[$iter];
										print OUT "$s $name ";
										if($#inputs < $limit)
										{
										 		$length = $#inputs;
										}
										else
										{
										 		$length = $limit - 1;
										}
										for($i=0;$i<=$length;$i++)
										{
												 $in  = shift(@inputs);
												 print OUT "$in";
										}
										print OUT "\n";
							 }
					}
					next;
		 }
		 print OUT $_;
}
close(IN);
close(OUT);
`mv $dummyfile $infile`;

