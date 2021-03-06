#! /usr/bin/perl -w


#########################################################################
# putActFactInOpt.pl
#########################################################################

# Written 9/26/2004 by Dinesh Patil 
# modified 10/18/2005 by Dinesh Patil 

# This script takes in the .power file containing the activity factors 
# generated by IRSIM runs and the modified spice file (used for optimization
# with ciropt) and adds the .POWER section in that spice file. One cant directly 
# include the .POWER section from the .power file due to case sensitive nature
# of ciropt vs the case insensitive nature of IRSIM!. 
#
# This script first changes the .power file itself to reflect the case sensitive names.
# It then uses the script putSectionInOpt.pl to put the .POWER section in the Optfile. 
#
# The script uses a dummy File whose  name is the input spice file prefixed with
# dummyActFact. This is deleted later and the result is the spice file with the 
# .POWER section included.
#
#########################################################################
# Load Libraries
#########################################################################
use File::Basename;
use Getopt::Long;
($prog) = fileparse($0);

# Check for proper number of arguments and extract args
if ($#ARGV != 1) { # $#ARGV is the number of command line arguments minus 1
    print STDERR "Usage: $prog Modified_spice_file  .power_file \n"; # $0 is the script name
    print STDERR "EXAMPLE:  $prog adder_64mod.sp  adder64.sp.power\n";
    print STDERR "A dummy file called  dummyActFact_Modified_spice_filename will be used\n";
    exit;
}
print "Running $prog\n";
$spfile = shift(@ARGV);
$powfile = shift(@ARGV);
$dumfile = $spfile . "dummy.actfact";

#record the duty factors from the .duty file first.
open (POW,"<$powfile") ||  die  ("Can't open $powfile : $!\n");
open (DUM,">$dumfile") ||  die  ("Can't open $dumfile : $!\n");

while(<POW>) {
   
   if(/\.POWER/ or /\.ENDS/) {
      next;
   }
   
   if(/^\*/) {
      print DUM $_;
      next;
   }
   s/;//;
   s/\s+//g;
   @tokens = split /:/, $_;
   $actFact{$tokens[0]} = $tokens[1] + 0;
}

close(POW);
open (SP,"<$spfile") ||  die  ("Can't open $spfile : $!\n");
$recordO = 0;
$recordPI = 0;
$recordSUB = 0;
$recordSUBst = 0;
while(<SP>)
{
#		 for recording nets internal to a CCC that have a cap and should be in energy equation
     if(/^\s*\.SUBCKT/)
		 {
					$recordSUB = 1;
		 }
		 if(/^\s*\.PI/)
		 {
					$recordPI = 1; # record the primary inputs.
		 }
		 if(/^\s*\.CONNECT/)
		 {
					$recordO = 1;   #record the outputs of all gates
		 }
		 if($recordSUB == 1)
		 {
					@tokens = split /\s*:\s*/, $_;
					@subtoken1 = split /\s+/, $tokens[0];
					shift(@subtoken1); #to remove the .SUBCKT label
					$gatetype = shift(@subtoken1);
					for($i=0;$i<=$#subtoken1;$i++)
					{
#							 print "$gatetype......$subtoken1[$i]\n";
							 $gateOutInfo{$gatetype}{$subtoken1[$i]} = 1;
					}
					$tokens[1] =~ s/\s*=\s*/=/g;
					@subtoken2 = split /\s+/, $tokens[1];
					for($i=0;$i<=$#subtoken2;$i++)
					{
#							 print "$gatetype......$subtoken1[$i]\n";
#							 record the input nets as well
							 if(! ( $subtoken2[$i] =~ /=/) )
							 {
										$gateInputInfo{$gatetype}{$subtoken2[$i]} = 1;
							 }
					}
					$recordSUB = 0;
					$recordSUBst = 1;
		 }
		 if($recordSUBst == 1)
		 {
					if(/^\s*(C[^\s]*)\s*:\s*([^\s]+)\s+([^\s]+)\s+c\s*=/)
					{ 
							 if((exists $gateOutInfo{$gatetype}{$2})
												 || (exists $gateOutInfo{$gatetype}{$3})
												 || (exists $gateInputInfo{$gatetype}{$3}))
							 {
										next;
							 }
							 elsif((lc($2) eq "gnd") or (lc($2) eq "vss") or (lc($2) eq "vdd"))
							 {
										$CapNet = $3;
							 }
							 else
							 {
										$CapNet = $2;
							 }
							 
							 push(@{ $gateIntInfo{$gatetype} },$CapNet);
#							 print "$1 $2 $3\n";
					}
		 }
		 if($recordO == 1)
		 {
					 if(/^[Vv]/)
					 {
								@tokens = split /\s*:\s*/, $_;
			  				@nets = split /\s+/, $tokens[1];
								for($i = 0; $i <= 1; $i++)
								{
			  							if(exists $actFact{lc($nets[$i])})
											{
													 $netActFact{$nets[$i]} = $actFact{lc($nets[$i])};
											}
			  							elsif(exists $actFact{$nets[$i]})
											{
													 $netActFact{$nets[$i]} = $actFact{$nets[$i]};
											}
											else
			  							{
													 print "offending place1\n";
			  									 die("The activity factor of net $nets[$i] is not reported\n");
			  							}
								}
					 }		
			  	 if(/^[Xx]/)
			  	 {
			  				@tokens = split /\s*:\s*/, $_;
			  				@nets = split /\s+/, $tokens[0]; #nets[1...$#] containts the output of every gate. 
#			  				print "$subtoken1[0] $subtoken2[0]\n";
			  				for($i = 1; $i <= $#nets; $i++)
								{
			  							if(exists $actFact{lc($nets[$i])})
											{
													 $netActFact{$nets[$i]} = $actFact{lc($nets[$i])};
											}
			  							elsif(exists $actFact{$nets[$i]})
											{
													 $netActFact{$nets[$i]} = $actFact{$nets[$i]};
											}
											else
			  							{
													 print "offending place2\n";
			  									 die("The activity factor of net $nets[$i] is not reported\n");
			  							}
								}
                @subtoken2 = split /\s+/, $tokens[2];
								$gatetype = shift(@subtoken2);
								if(exists $gateIntInfo{$gatetype})
								{
										 @intNames = @{ $gateIntInfo{$gatetype}};
										 for($i=0;$i<=$#intNames;$i++)
										 {
#													print "Reached Here $intNames[$i]\n";
													$intNet = $nets[0].".".$intNames[$i];
													$recordNet = $nets[0]."!!!".$intNames[$i];
                          if(exists $actFact{lc($intNet)})
													{
                          		 $netActFact{$recordNet} = $actFact{lc($intNet)};
                          }
                          elsif(exists $actFact{$intNet})
                          {
                          		 $netActFact{$recordNet} = $actFact{$intNet};
                          }
                          else
                          {
													 print "offending place3\n";
                          		 die("The activity factor of net $intNet is not reported\n");
													}
										 }
								}
			  	 }
		 }
		 if($recordPI == 1)
		 {
			  	 if(/^name/)
			  	 {
			  				@tokens = split /\s*,\s*/, $_;
			  				@inet = split /=/, $tokens[0]; #inet[1] containts the primary input name. 
#			  				print "$subtoken1[0] $subtoken2[0]\n";
			  				if(exists $actFact{lc($inet[1])})
								{
										 $netActFact{$inet[1]} = $actFact{lc($inet[1])};
								}
								elsif(exists $actFact{$inet[1]})
								{
										 $netActFact{$inet[1]} = $actFact{$inet[1]};
								}
								else
			  				{
													 print "offending place4\n";
			  						 die("The activity factor of net $inet[1] is not reported\n");
			  				}
			  	 }
		 }
		 if(/^\s*\.ENDS/)
		 {
					$recordO = 0;
					$recordPI = 0;
					$recordSUBst = 0;
		 }
}
print DUM ".POWER\n";
for $key ( keys %netActFact)
{
 		if(exists $netActFact{$key})
 		{
#				 print "$key......\n";
				 if($key =~ /([^\s]+)!!!([^\s]+)/)
				 {
							print DUM "$1 $2 : $netActFact{$key} ;\n";
				 }
				 else
				 {
							print DUM "$key : $netActFact{$key} ;\n";
				 }
 		}
}
print DUM ".ENDS\n\n";
close(SP);
close(DUM);
$scotHome = $ENV{ 'SCOT_HOME_DIR' };
$pyHome   = $scotHome . '/pys';
$psioPy   = $pyHome   . '/putSectionInOpt.py';
system("$psioPy $spfile $dumfile POWER");
`rm -f $dumfile`;
