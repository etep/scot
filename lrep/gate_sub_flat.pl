#! /usr/bin/perl -w 


#########################################################################
# gate_sub_flat.pl
#########################################################################

# Written 2/2/2003 by Dinesh Patil 
# modified (line 146) 4/20/05 by Dinesh Patil

# This script changes the hierarchy in spice netlist (generated by sue) to 
# flatten which ever subckt is specified in the arguments. If "mainlist" is 
# specified, then the script flattens the entire file to the CCC level.
# This is then fed to sue_spice.pl. Hence this program is redundant if the 
# circuit is alredy in proper form.

# There are three kinds of subcircuits: CCCs, subcircuits containing only CCCs
# and then the one containing transistors and other subckts. 
# This script can't handle the mixed type 
# and hence the moment they are detected this script will give and error and die
# Now all the subcircuits and their important parameters, like the name, ports
# any width parameters and their type are maintained in a hash indexed by thier
# name. Its called %subckts.
# The algorithm first gathers all the subckts and also the main netlist 
# in hash %subckts. Then it takes the main netlist and keeps on scanning it for
# any possible non-CCC subcircuits which it then replaces with the statements
# from that subckt (with appropriate modification to show hierarchy...). It
# can do this in one go with recursion or in a loop..Looping is used.
# Evidently here, our atomic structure is the CCC. Hence if a subckt 
# contains only CCCs its already flat. It will be expanded in the main netlist.
# And the subckts that are CCCs are not touched.



#########################################################################
# Load Libraries
#########################################################################
use File::Basename;
use Getopt::Long;

($prog) = fileparse($0);

# Check for proper number of arguments and extract args
if ($#ARGV != 2) { # $#ARGV is the number of command line arguments minus 1
    print STDERR "Usage: $prog heirarchical_spice_infile outfile subckt_name/\"mainlist\"\n"; # $0 is the script name
    print STDERR "EXAMPLE:  $prog foo_in.sp foo_in.spi mainlist\n";
    exit;
}
print "Running gate_sub_flat.pl on $ARGV[0]\n";

$datafile = $ARGV[0];
$outfile  = $ARGV[1];
$level    = $ARGV[2];
$intfile  = $outfile.".tmp";
$delim = "__";




#to remove the + line extensions used by spice to format the input if there its a long line.
#uses the internal file called $outfile.".tmp"
open (DATA, "<$datafile" ) ||  die  ("Can't open $datafile : $!\n");
open (INT,  ">$intfile"  ) ||  die  ("Can't open $intfile : $!\n");
$record = <DATA>; 
while(<DATA>)
{
#	print "Line: $record ";
	if(/^\+(.*)/)
	{
		$curr = $1;
		$record =~ /(.*)\n/s;
		$record = $1.$curr."\n";
#		print " KKK $record , LLL $curr";
	}
	elsif(/^\*\+/ and $record =~ /^\*/)
	{
		$_ =~ /^\*\+(.*)/; # has to be done separately, seems to not work in the elsif statement above.
		$currComment = $1;
		$record =~ /(.*)\n/s;
		$record = $1.$currComment."\n";
	}
	else
	{
		print INT "$record";
		$record = $_;
	}
}
print INT "$record";

close(DATA);
close(INT);

$datafile = $intfile;
	

#####################################################################################################
##RECORD THE CIRCUIT DETAILS IN A HASH

$is_subckt = 0;
$is_ccc = 0;
$is_group_ccc = 0;
$lineno = 0;
$firstLineIsCap = 0;
#$doNotUniquify = 0;
open (DATA,"<$datafile") ||  die  ("Can't open $datafile : $!\n");
push @{ $subckts{mainlist}{statements} }, "**MAINLIST BEGINS HERE\n"; #to simulate the fact that subckts usually have a title line.
while(<DATA>)
{
	$lineno++;
	s/\s*=\s*/=/g;
	if(/^\.SUBCKT/i)
	{
		$first_line = 1; # to record that this is the first line of the subckt
		$subckt_def = $_; #record this line, if its a ccc we will print it
		$is_subckt = 1; #inside a subckt
		@tokens = split /\s+/, $_;
#		$cur_subckt_name = $tokens[1]; #subckt name
		push @ckt_types, $tokens[1];
		$i = 2;
		@ports = [];
		shift(@ports);
		@params = [];
		shift(@params);
#		print "HHHHHHHH $tokens[$#tokens] $tokens[$#tokens-1]\n\n";
		while($i <= $#tokens && !($tokens[$i] =~ /.*=.*/))
		{
			push @ports, $tokens[$i];
			$i += 1;
		}
		while($i <= $#tokens)
		{
			push @params, $tokens[$i];
			$i += 1;
		}
		$subckts{$tokens[1]}{ports} = [@ports];
		$subckts{$tokens[1]}{params} = [@params];
		if(!exists $subckts{$tokens[1]}{dnu})
		{
				 $subckts{$tokens[1]}{dnu} = 0;  #start with the notion that there is uniquifying.
		}
		next;
	}
	if(/^\.ENDS/i)
	{	
		if($is_ccc == 1)
		{
				 print "Checking the validity of CCC $tokens[1]\n";
				 checkCCCvalid($tokens[1]);
# do various checks on this CCC to ensure that its a valid CCC in terms of inputs 
# going to gates and outputs coming from source drain and so on.
          }	 
		$is_subckt = 0;
		$is_group_ccc = 0;
		$is_ccc = 0;
		next;
	}
	if(/^\*\s*glbcnst\s*:\s*DoNotUniquify\s*(.*)\s*;\s*$/)
	{
#		print "Reached here\n";
		if($first_line == 1)
		{
			push @{ $subckts{$tokens[1]}{statements} }, $subckt_def;	
			$first_line =0;
		}
		$elements = $1;
#		print "$elements ****\n";
		if($elements eq "")
		{
				 print "DoNotUniquify has no arguments: All subckts will be uniquified\n";
		}
		else
		{
				 @deUniq = split /\s+/, $elements;
				 for ($l=0;$l<=$#deUniq;$l++)
				 {
							print "DeUniquifying $deUniq[$l]\n";
							$subckts{$deUniq[$l]}{dnu} = 1;
				 }
		}
	}
	if(/^\*basic block/)
	{
		if($first_line == 1)
		{
			push @{ $subckts{$tokens[1]}{statements} }, $subckt_def;	
			$first_line =0;
		}
			$subckts{$tokens[1]}{type} = 3;
			$subckts{$tokens[1]}{used} = 0;
	}
	if((/^[XRCLVxrclv]/ or /^\*/) and $is_subckt == 1)
	{
		if($first_line == 1)
		{
			$subckts{$tokens[1]}{type} = 2;
			$subckts{$tokens[1]}{used} = 0;
			push @{ $subckts{$tokens[1]}{statements} }, $subckt_def;	
			$first_line =0;
		}
		if($firstLineIsCap == 1)
		{
			$subckts{$tokens[1]}{type} = 2;
			$subckts{$tokens[1]}{used} = 0;
			$firstLineIsCap = 0;
		}
		if(/^[Xx]/)
		{
				 $is_group_ccc = 1;
		}
		if(/^\*/)
		{
				 push @{ $subckts{$tokens[1]}{statements} }, $_;
				 next;
		}
		push @{ $subckts{$tokens[1]}{statements} }, $_;
		if($is_ccc == 1 and $is_group_ccc == 1)
		{
			print "err1: A subckt is found that contains both, \n";
			print "transistors and gate icons, not allowed \n";
			die("Offending statement : $_");
		}
#		print "hehehe $subckts{$tokens[1]}{statements} \n"
		next;
	}
	if((/^[RCLrcl]/) and $is_subckt == 1)
	{
		if($first_line == 1)
		{
#			$subckts{$tokens[1]}{type} = 3;
#			$subckts{$tokens[1]}{used} = 0;
			$first_line =0;
			push @{ $subckts{$tokens[1]}{statements} }, $subckt_def;	
			$firstLineIsCap = 1;
		}
		if(/^[Cc]/)
		{
#				 $_ = "*SUBCKT-".$_;
				 @fields = split /\s+/, $_;
				 if($#fields > 3)
				 {
							die("Cap statement has too many arguments, only 4 allowed\n");
				 }
				 if(($fields[1] =~ /\[(.*):(.*)\]/s) and ($fields[2] =~ /\[(.*):(.*)\]/s))
				 {
							die("The cap $fields[0] seems to be connected between two busses\n Terminated\n");
				 }
				 if(($fields[1] =~ /\[(.*):(.*)\]/s) or ($fields[2] =~ /\[(.*):(.*)\]/s))
				 {
						 if(($fields[1] =~ /\[(.*):(.*)\]/s))
						 {
								 $signet = $fields[1];
								 $powernet = $fields[2];
						 }
						 else
						 {
								 $signet = $fields[2];
								 $powernet = $fields[1];
						 }
					      if($1>$2)
					 	 {
						 		$start_bus = $2;
						 		$end_bus = $1;
						 }
						 else
						 {
						 		$start_bus = $1;
						 		$end_bus = $2;
						 				 
						 }
						 for($bus=$start_bus;$bus<=$end_bus;$bus++)
						 {
						 		$capName = $fields[0]."[$bus]";
						 		$tmp = $signet;
						 		$tmp =~ s/\[(.+):(.+)\]/[$bus]/g;
						    $stt =  "$capName $powernet $tmp $fields[3]\n";
							  push @{ $subckts{$tokens[1]}{statements} }, $stt;
						 }
				 }
				 else
				 {
							push @{ $subckts{$tokens[1]}{statements} }, $_;
				 }
				 next;
		}
		push @{ $subckts{$tokens[1]}{statements} }, $_;
	}
	if((/^[Mm]/) and $is_subckt == 1)
	{
		if($first_line == 1)
		{
			$subckts{$tokens[1]}{type} = 1;
			push @{ $subckts{$tokens[1]}{statements} }, $subckt_def;	
			$first_line =0;
		}
		if($firstLineIsCap == 1)
		{
			$subckts{$tokens[1]}{type} = 1;
			$firstLineIsCap = 0;
		}
		$is_ccc = 1;
		push @{ $subckts{$tokens[1]}{statements} }, $_;
		if($is_ccc == 1 && $is_group_ccc == 1)
		{
			print "err2: This subckt at $lineno contains both \n";
			print "transistors and gates, not allowed \n";
			die;
		}
		next;
	}
	if(/^[XRLVxrlv]/ and $is_subckt == 0)
	{
		push @{ $subckts{mainlist}{statements} }, $_;
		next;
	}
	if(/^[Cc]/ and $is_subckt == 0)
	{
				 @fields = split /\s+/, $_;
				 if($#fields > 3)
				 {
							die("Cap statement has too many arguments, only 4 allowed\n");
				 }
				 if(($fields[1] =~ /\[(.*):(.*)\]/s) and ($fields[2] =~ /\[(.*):(.*)\]/s))
				 {
							die("The cap $fields[0] seems to be connected between two busses\n Terminated\n");
				 }
				 if(($fields[1] =~ /\[(.*):(.*)\]/s) or ($fields[2] =~ /\[(.*):(.*)\]/s))
				 {
						 if(($fields[1] =~ /\[(.*):(.*)\]/s))
						 {
								 $signet = $fields[1];
								 $powernet = $fields[2];
						 }
						 else
						 {
								 $signet = $fields[2];
								 $powernet = $fields[1];
						 }
					   if($1>$2)
					 	 {
						 		$start_bus = $2;
						 		$end_bus = $1;
						 }
						 else
						 {
						 		$start_bus = $1;
						 		$end_bus = $2;
						 				 
						 }
						 for($bus=$start_bus;$bus<=$end_bus;$bus++)
						 {
						 		$capName = $fields[0]."[$bus]";
						 		$tmp = $signet;
						 		$tmp =~ s/\[(.+):(.+)\]/[$bus]/g;
						    $stt =  "$capName $powernet $tmp $fields[3]\n";
							  push @{ $subckts{mainlist}{statements} }, $stt;
						 }
				 }
				 else
				 {
							push @{ $subckts{mainlist}{statements} }, $_;
				 }
				 next;
	}
	if(/^\.GLOBAL/i)
	{
		@globals = split /\s+/, $_;
		shift(@globals);
		for $net (@globals)
		{
			$global{lc($net)} = 1;
		}
	}
	if($is_ccc == 1 && $is_group_ccc == 1)
	{
		print "err3: This subckt at $lineno contains both \n";
		print "transistors and gates, not allowed \n";
		die;
	}
}
close(DATA);


###PRINT THE CONTENTS OF %subckt...
#	for $ckt (@ckt_types)
#		{
#			print "hash keys $ckt $subckts{$ckt}{type} \n";
#			for $port ( @{$subckts{$ckt}{ports}} )
#			{
#				print  "array keys: $port\n";
#			}
#			print ".ENDS\n\n";
#		}
#
#die;



##############################################################################################
#Process the main netlist now to expand the subckts.
$not_expand = 0;
while($not_expand == 0)
{
	@new_netlist = [];
	shift(@new_netlist); #done to remove the spurious first element 
	$not_expand = 1; #flag to indicate that no futher expansion is necessary
	$which_line = 0;
	for $line (@{$subckts{$level}{statements}} )
	{
	  #skip if not a subckt.
		if(($line =~ /^[RrCcLlVv]/s) or ($line =~ /^\*/s) or $which_line == 0)
		{
			$which_line = 1;
			push @new_netlist, $line;
			next;
		}
		if($line =~ /^[Xx]/s)
		{
			
			@parts = split /\s+/, $line;
			$inst = shift(@parts);
			$var_prefix = $inst; #default is to assume that the instance is uniquified
			$choose = 0;
#get the subckt type (called ccc_type below)
			for($i=0;($i<=$#parts && $choose == 0);$i++)
			{
#				print "parts ::: $parts[$i] \n";
				if(($parts[$i] =~ /=/s) && $choose == 0)
				{
					$choose = 1;
					$ccc_type = $parts[$i-1];
				}
				elsif($i == $#parts && $choose == 0)
				{
					$choose = 1;
					$ccc_type = $parts[$i];
				}
			}
			if($subckts{$ccc_type}{dnu} == 1)
			{
					 $var_prefix = "U".$ccc_type;
			}
			else
			{
					 if($line =~ /VarName=\s*([^\s]+)\s*$/)
					 {
								$var_prefix = $1;
					 }
			}
#			print "line is :: $line";
#			print "HUHUHUH $inst $ccc_type ::: $subckts{$ccc_type}{type} \n\n"; 
			if ($subckts{$ccc_type}{type} == 1) #its a CCC
			{
				if($subckts{$ccc_type}{dnu} == 1)
				{
						 print "Error: DoNotUniquify cannot be applied to a CCC - $ccc_type \n";
						 print "For applying DoNotUniquitfy, wrap this CCC in a subcircuit\n";
						 exit;
				}
				if(!($line =~ /=/s))
				{
					$line =~ /(.*)\n/s;
					$line = $1;
					for $param (@{$subckts{$ccc_type}{params}})
					{
						$line = $line." ".$param;
					}
					$line = $line."\n";
				}
				push @new_netlist, $line;
				next;
			}
			if ($subckts{$ccc_type}{type} == 2) #it is not a CCC .
			{
				@pins = @{$subckts{$ccc_type}{ports}};
#copy the ports in a hash to exchange the names....
				%exchange = ();
				for $i (0 .. $#pins)
				{
					$exchange{$pins[$i]} = $parts[$i];
#					print "exchange $subckts{$ccc_type}{ports}[$i] with $parts[$i] \n";
				}
				$not_expand = 0; # at least one subckt to expand, so keep the loop.
				$subckts{$ccc_type}{used} += 1;
				if($subckts{$ccc_type}{used} > 2 && $subckts{$ccc_type}{dnu} != 1)
				{
					print "Uniquify warning: The subckt $ccc_type is repeated without DeUniquifying\n"; 
					print "The script will uniquify the subckt based on its name\n";
#					print "The back_annotated sizing in SUE might not be the right size for this Subckt\n";
				}
# record the ports in the exchange hash to find and exchange them,,the rest of 
# the nets will be appended by the current sub ckts name using a "__" as a delimiter. 
				@block = @{$subckts{$ccc_type}{statements}};
				shift(@block); # to remove the subckt declaration. 
				for $insert (@block)
				{
					@fields = split /\s+/, $insert;
					if($fields[$#fields] =~ /VarName=(.+)\s*$/)
					{
							 pop(@fields);
							 $var_name =  $1;
					}
					else
					{
							 $var_name =  $fields[0];
					}
					$leave = 0;
#					$fields[0] =~ /[XRCL](.*)/s;
#					$fields[0] = $inst."/".$1;
					if($fields[0] =~ /^[Xx]/)
					{
						$fields[0] = $inst.$delim.$fields[0];
					  $var_name =  $var_prefix.$delim.$var_name;
					}
					elsif($fields[0] =~ /^([RrCcLlVv])/)
					{
						$fields[0] = $1.$inst.$delim.$fields[0];
						$var_name = $fields[0];
					}
#					print "nananan $ccc_type and  $fields[0] and $var_name\n\n";
					$num = 0;
					while($leave == 0)
					{
						$num++;
						if(($num >= $#fields) or ($fields[$num+1] =~ /=/s)
								or ($fields[0] =~ /^[RCLrclVv]/ and $num==3))
						{
							$leave = 1;
							next;
						}
						if(exists $exchange{$fields[$num]})
						{
							$fields[$num] = $exchange{$fields[$num]};
							next;
						}
						if(exists $global{lc($fields[$num])})
						{
							next;
						}
						$fields[$num] = $inst.$delim.$fields[$num]; 
					}
					$newline = join " ", @fields;
					if($fields[0] =~ /^([RrCcLlVv])/)
					{
							 $newline = $newline."\n";
					}
					else
					{
							 $newline = $newline." VarName=$var_name\n";
					}
#					print "this is the line $newline";
					push @new_netlist, $newline;	
				}
			}
		}
	}
#	for $statt (@new_netlist)
#	{
#		print $statt;
#	}
	@{$subckts{$level}{statements}} = @new_netlist;
#	print "\n\n";
#	for $statt (@{$subckts{$level}{statements}})
#	{
#		print $statt;
#	}
}


##################################################################################################
##PRINT THE NEW FILE WITH GATE FLAT NETLIST.........
open (DATA,"<$datafile") ||  die  ("Can't open $datafile : $!\n");
open (OUT,">$outfile")   ||  die  ("Can't open $outfile : $!\n");

$no_other_lines = 0; # This variable is 0 to print the beginning all the lines.
# When subckt starts it is turned on so that its not used in the repeated 
# reading of that file (to flatten the heirarchy)

$put_new_stuff = 0;
$is_subckt = 0;
$in_mainlist =0;
while(<DATA>)
{
	if(/^\.SUBCKT/i)
	{
			 if (/$level/) # Now put the expanded subckt
			 {
						for $net_line (@{$subckts{$level}{statements}})
						{
								 print OUT $net_line;
						}
						print OUT ".ENDS\n\n";
			 }
			 $is_subckt = 1;
			 $no_other_lines = 1;
			 if($put_new_stuff == 0)
			 {
						$put_new_stuff = 1;
			 }
			 next;
	}
	if(/^\.ENDS/i)
	{
		$is_subckt = 0;
		next;
	}
	if($is_subckt == 1)
	{
		next;
	}
	if(/^[XxRrCcLlVv]/)
	{
		$in_mainlist = 1;
		next;
	}
	if($no_other_lines == 0 )
	{
		print OUT $_;
		next;
	}
	if($in_mainlist == 0)
	{
		next;
	}
	if($put_new_stuff == 1)
	{
#		print "am here\n";
		$put_new_stuff = 2;
# put the new stuff
		for $ckt (@ckt_types)
		{
			if(!($ckt eq "mainlist") and $subckts{$ckt}{type} == 1)
			{
				for $net_line (@{$subckts{$ckt}{statements}})
				{
					print OUT $net_line;
				}
				print OUT ".ENDS \$ $ckt\n\n";
				
			}
		}
		for $net_line (@{$subckts{mainlist}{statements}})
		{
			print OUT $net_line;
		}			
	}
	if($in_mainlist == 1 && $put_new_stuff ==2)
	{
		print OUT $_;
		next;
	}
}	
close(DATA);
close(OUT);
#`rm -f $datafile`;



##################################################################################################
##CCC NETLIST CHECKER for valid representations.


sub checkCCCvalid
{
		 my $ccc_name = shift(@_);
# gather the inputs and the outputs in a hash
     my @substt = @{$subckts{$ccc_name}{statements}};
     my $declaration = $substt[0];
		 my @sections = split /\s+/, $declaration;
		 my @outputs = {};
		 my $inputs = {};
		 for($i=2;$i<=$#sections;$i++)
		 {
					if(!($sections[$i] =~ (/=/)) and ($sections[$i] =~ /^[A-Z_0-9]/))
					{
							 print "The i/o name $sections[$i] of CCC $ccc_name is not valid\n";
							 print "The output names must start with [a-h] and the inputs must start with [i-z]\n";
							 print "Please modify the names in SUE so that you have outputs preceding inputs \n";
							 print "in the port list of a ccc subcircuit specification.\n";
							 exit;
					}
					if(!($sections[$i] =~ /=/) and ($sections[$i] =~ /^[a-h]/))
					{
#							 print "output: $sections[$i]\n";
							 $outputs{$sections[$i]} = 1;
					}
					if(!($sections[$i] =~ /=/) and ($sections[$i] =~ /^[i-z]/))
					{
#							 print "input: $sections[$i]\n";
							 $inputs{$sections[$i]} = 1;
					}
		 }
		 for($i=1;$i<$#substt;$i++)
		 {
					if(($substt[$i] =~ /^[Cc]/) or ($substt[$i] =~ /^\*/))
					{
							 next;
					}
					if(($substt[$i] =~ /^[RrLlVv]/))
					{
							 print "ERROR: No Voltage source or resistor is allowed inside a CCC\n";
							 print "       Offending CCC $ccc_name.\n";
							 exit;
					}
#					print "this statement $substt[$i] \n";
					@sections = split /\s+/, $substt[$i];
# the zeroth element is the name 
# 1 = drain
# 2 = gate 
# 3 = source
# 4 = body.
				  if((exists $inputs{$sections[1]}) or (exists $inputs{$sections[3]}) or (exists $inputs{$sections[4]}))
					{
							 print "ERROR: An input is connected to source,drain or body of\n";
							 print "       the transistor $sections[0] in CCC $ccc_name.\n";
							 print "       Partial CCCs are not allowed\n";
							 exit;
					}
				  if((exists $outputs{$sections[2]}) or (exists $outputs{$sections[4]}))
					{
							 print "ERROR: An output is connected to gate or body of\n";
							 print "       the transistor $sections[0] in CCC $ccc_name.\n";
							 print "       Source-drain loops inside CCCs are not allowed\n";
							 exit;
					}
		 }
}
	

