#! /usr/bin/perl -w 


#########################################################################
# sue_spice.pl
#########################################################################

# Written 6/5/2003 by Dinesh Patil 

# This script changes the spice file generated from sue (followed by gate_sub_flat.pl) 
# to the one needed by the optimizer. This includes putting in the global parameters and 
# constraints as well as other small things like the syntax etc..
# The input file is typically a .spi file. The output is a .sp file with 
# the same heading as the .spi file

#########################################################################
# Load Libraries
#########################################################################
use File::Basename;
use Getopt::Long;

($prog) = fileparse($0);

# Check for proper number of arguments and extract args
if ($#ARGV < 2 || $#ARGV > 3) {
   # $#ARGV is the number of command line arguments minus 1
   print STDERR "Usage: $prog glbparamfile spice_infile result_file <opt_file>(if you have it)\n"; # $0 is the script name
   print STDERR "EXAMPLE: $prog glb_par.sp foo_in.spi new_spice.sp <foo.opt>\n";
   print STDERR "The opt file is optional and can be included later manually\n";
   exit;
}

print "Running sue_spice.pl\n";
#$LAMBDA = "0.03u";
$glbparfile = $ARGV[0];
#$glbcnstrfile = $ARGV[1];
$infile = $ARGV[1];

@tokens = split /\./, $infile;
pop(@tokens); # remove the .sp , .spi etc extension.
$filePrefix = join ".", @tokens;
$outfile = $ARGV[2];

if($#ARGV == 3) {
   $optfile = $ARGV[3];
   $optfileDefined = 1;
}
else {
   $optfileDefined = 0;
}
		 
#print "$outfile $infile $tokens[0] $tokens[1] \n";
$connect = ".CONNECT";
# these are other global variables that I might be using in future to make the 
# script more intelligent 
$pi = ".PI";
$po = ".PO";
$ends = ".ENDS";
$glbcnstr = ".GLBCNSTR";
$global   = ".GLOBAL";

 #This is ":" for multiple output ccc, but for backward compatibility, we have to have nothing.
$outInSeparator = ":";

system( "cp $glbparfile $outfile" );

open (IN,"<$infile") ||  die  ("Can't open $infile : $!\n");

open( OUT,    ">>$outfile")           || die  ("Can't open $outfile : $!\n");
open( PI,     ">".$filePrefix.$pi)    || die  ("Can't open $filePrefix$pi : $!\n");
open( PO,     ">".$filePrefix.$po)    || die  ("Can't open $filePrefix$po : $!\n");
open( GLCNST, ">".$filePrefix.".glc") || die  ("Can't open $filePrefix.glc : $!\n");

print PI ".PI\n";
print PO ".PO\n";
print GLCNST "$glbcnstr\n";
print OUT "\n";


$a = 1;
$inSub = 0;

#this loop is to skip the lines until a subckt is found. 
while( $a != 0 ) {
   $_ = <IN>;
   if( /^\s*\.SUBCKT/ or /^\s*\.subckt/ ) {
      $inSub = 1;
      $a = 0; #exit the loop once the subckt is found.
      s/\s*=\s*/=/g;
      # s/@//g; # remove @ from Electric default names
      @tokens = split /\s+/, $_;
      $subcktname = $tokens[1];
      # print "$subcktname is the name\n";
      $i=0;
      # find out the number of outputs for this CCC. typically 1, but could be multiple
      for( $j=2; $j <= $#tokens;$j++ ) {
         if( $tokens[$j] =~ /^([^\s]+)=([^\s]+)$/ ) {
            $parameters{$subcktname}{$1} = $2;
         }
         else {
            if( $tokens[$j] =~ /^[A-Z_0-9]/ ) {
               print "ERROR: $tokens[$j] : all the input - output ports of the ccc should\n"; 
               print "       start with small letters\n";
               die;
            }
            elsif( $tokens[$j] =~ /^[a-h]/ )  {
               # detect the number of outputs to put the delimiter.
               $i++;
            }
         }
      }
      $tokens[$#tokens] = $tokens[$#tokens].";";
      $tokens[$i + 1] = $tokens[$i + 1]." $outInSeparator";
      $state = join " ", @tokens;
      print OUT "$state\n";
      # record this number in a hash
      $outputs{$subcktname} = $i;
   }
}


$notTopCell = 0; # flag to indicate that we are at the beginning of the mainlist 
$inTopCell  = 0; # flag to indicate we are inside the mainlist

#%capnames = ();
while( <IN> ) {
   s/\s*=\s*/=/g;
   #	s/@//g; # remove @ from electric default names
   if( /^\s*\.SUBCKT/ or /^\s*\.subckt/ ) {
      $notTopCell = 0;
      $inTopCell = 0;
      $inSub = 1;
      @tokens = split /\s+/, $_;
      $subcktname = $tokens[1];
      $i=0;
      # find out the number of outputs for this CCC. typically 1, but could be multiple
      for( $j=2; $j<=$#tokens; $j++ ) {
         if( $tokens[$j] =~ /^([^\s]+)=([^\s]+)$/ ) {
            $parameters{$subcktname}{$1} = $2;
         }	
         else {
            if    ( $tokens[$j] =~ /^[A-Z_0-9]/ ) {
               print "ERROR: $tokens[$j] : all the input and output ports of the ccc should\n"; 
               print "       start with small letters [a-z]\n";
               die;
            }
            elsif ( $tokens[$j] =~ /^[a-h]/     ) {
               $i++;
            }
         }
      }
      $tokens[$#tokens] = $tokens[$#tokens].";";
      $tokens[$i + 1] = $tokens[$i + 1]." ".$outInSeparator;
      $state = join " ", @tokens;
      print OUT "$state\n";
      # record this number in a hash
      $outputs{$subcktname} = $i;
      next;
   }
   
   if( /^\s*[Mm]/ and $inSub == 1 ) {
      # its a transistor..have the type changed to nmos and pmos. (6th argument)
      # first check that if the Width is parameterized then its in parenthesis!
      if( /W.*='+(.+)'+/ ) {
         $parWidth = $1;
         if( !($parWidth =~ /\(.+\)/) and $parWidth =~ /[\+\*\^\/]+/ ) {
            die ("Parameterized widths should be in parenthesis : e.g. WN='(expression)'; see in statement : $_");
         }
         s/'//g;
   	}
      @tokens = split /\s+/, $_;
      if    ( $tokens[5] =~ /^[nN]/ ) {
         $tokens[5] = "nmos";
      }
      elsif ( $tokens[5] =~ /^[pP]/ ) {
         $tokens[5] = "pmos";
      }
      else {
         die("ERROR: the transistor type could not be determined! in $subcktname\n");
      }
      $state = join " ", @tokens;
      print OUT "$state;\n";
      next;
   }
   
   if( /$ends.*/ ) {
      print OUT $ends;
      print OUT "\n";
      $inSub = 0;
      $notTopCell = 1;
      next;
   }
   
   if( /^\s*$/ ) {
      print OUT $_ ;
      next;
   }
   
   if( $notTopCell == 1 && /^[RrCcXxVvLl]/ )  {
      # beginning of anything that belongs to the topCell
      print OUT "$connect\n";
      $notTopCell = 0;
      $inTopCell = 1;
   }
   
   if( /^[Xx]/ ) {
      @tokens = split /\s+/, $_;
      $ccc_inst = $tokens[0];
      $var_name = $ccc_inst ;
      $ccc_found = 0;
      $xstatement = "";
      for($i = 0; ($i <= $#tokens-1); $i +=1 ) {
         
         if( ( $tokens[$i+1] =~ /.*=.*/ ) && $ccc_found == 0 ) {
            if( /VarName=\s*(.+)\s*$/ ) {
               $var_name = $1;
               if( $var_name eq $ccc_inst ) {
                  $xstatement =  $xstatement." : $tokens[$i]";
               }
               else {
                  $xstatement =  $xstatement." : $tokens[$i] : $var_name"; 
               }
            }
            else {
               $xstatement =  $xstatement." : $tokens[$i]";
            }
            
            $ccc = $tokens[$i]; #name of the ccc type.
            #	print "The CCC Type is $ccc\n";
            $ccc_found = 1;
            # This hash is to record if the uniquified cell's local constraints have been put. If they 
            # have been formulated, they are not formulated for another instance, as they will be the same.
            if(!(exists $UniqLocalCnstWritten{$var_name})) {
               $UniqLocalCnstWritten{$var_name} = 0;
            }
            # pry local constraints associated with this CCC in .GLBCNST section.
            if( (exists $localConst{$ccc}) && $UniqLocalCnstWritten{$var_name} == 0 ) {
               @const = @{ $localConst{$ccc}};
               # print GLCNST "$ccc_inst $ccc somthing\n";
               for( $k=0; $k <= $#const; $k++ ) {
                  $const[$k] =~ s/.*:\s*//;
                  $constraint =  $const[$k];
                  $constraint =~ s/(W[A-Za-z0-9]+)/$var_name\.$1/g;  
                  $constraint =~ s/(Vth[A-Za-z0-9]+)/$var_name\.$1/g;  
                  print GLCNST "$constraint";
               }
               if( !($var_name eq $ccc_inst) ) {
                  # print "The local cnst is stopped for $ccc in instance $ccc_inst \n ";
                  $UniqLocalCnstWritten{$var_name} = 1;
               }
      		}
      	}
      	elsif( ($tokens[$i] =~ /(W[^\s]*)\s*=.*/s) and $ccc_found == 1 ) {
            $curr_width = "$var_name."."$1";
            # print OUT "$1=$curr_width ";
            if( !( $tokens[$i] =~ /VarName\s*=/) ) {
               $xstatement =  $xstatement." $tokens[$i]";
            }
            $hash_width{$curr_width} = 1;
         }
         else {
            if( !( $tokens[$i] =~ /VarName\s*=/) ) {
               $xstatement =  $xstatement." $tokens[$i]";
            }
         }
      }
      if( ($tokens[$i] =~ /(W[^\s]*)\s*=.*/s) and $ccc_found == 1 ) {
      	$curr_width = "$var_name."."$1";
      	$hash_width{$curr_width} = 1;
      }
      # print OUT "$1=$ccc_inst."."$1;\n";
      if( !( $tokens[$i] =~ /VarName\s*=/) ) {
         $xstatement =  $xstatement." $tokens[$i]\n";
      }
      @tokens = split /\s+/, $xstatement;
      shift(@tokens); # since the first token is empty
      # print "HAHAHAHA $ccc has $outputs{$ccc} outputs\n";
      $tokens[$outputs{$ccc}] = $tokens[$outputs{$ccc}]." ".$outInSeparator;
      %listedParam = ();
      # print "Look at $ccc $ccc_inst parameter defs\n";
      for( $i=0; $i<=$#tokens; $i++ ) {
         if( $tokens[$i] =~ /^([^\s]+)=([^\s]+)$/ ) {
            $listedParam{$1} = $2;
         }
      }
   	for $key (keys %{ $parameters{$ccc} }) {
         if(exists $parameters{$ccc}{$key}) {
            # print "$ccc .....$key ...$parameters{$ccc}{$key} \n";
            if( !(exists $listedParam{$key}) ) {
               $newtoken = $key."=".$parameters{$ccc}{$key};
               push(@tokens, $newtoken);
            }
         }
   	}					
   	$xstatement = join " ", @tokens;
   	print OUT "$xstatement;\n";
   	next;
   }
   
   if( /^[Cc]/ ) {
      s/'//g;
      @tokens = split /\s+/, $_;
      if    ( isVddOrGnd($tokens[1]) == 0 && isVddOrGnd($tokens[2]) == 0 ) {
         print "$tokens[0] : Recording a potential coupling cap\n";
         $powernet = $tokens[1];
         $signet = $tokens[2];
   	}
   	elsif ( isVddOrGnd($tokens[1]) == 1 && isVddOrGnd($tokens[2]) == 1 ) {
         print "The cap $tokens[0] is between power terminals, hence ignored\n";
         next;
   	}
   	elsif ( isVddOrGnd($tokens[1]) == 1 ) {
         $powernet = $tokens[1];
         $signet = $tokens[2];
   	}
   	elsif ( isVddOrGnd($tokens[2]) == 1 ) {
         $powernet = $tokens[2];
         $signet = $tokens[1];
   	}
   	if( $tokens[$#tokens] =~ /^([0-9.]+)[fF][fF]*$/ ) {
         $capValue = $1;
   	}
   	else {
         $capValue = $tokens[$#tokens];
   	}
      # check if the cap is on a bus and if so, expand it to all the lines of the bus.
   	if( $signet =~ /\[(.*):(.*)\]/s ) {
         # commented cause this is handled in the gate_sub_flat.pl routine
         # if($1>$2) {
         #    $start_bus = $2;
         #    $end_bus = $1;
         # }
         # else {
         #    $start_bus = $1;
         #    $end_bus = $2;
         # }
         # for($bus=$start_bus;$bus<=$end_bus;$bus++) {
         #    $capName = $tokens[0]."[$bus]";
         #    $tmp = $signet;
         #    $tmp =~ s/\[(.*):(.*)\]/[$bus]/g;
         #    print OUT "$capName $outInSeparator $powernet $tmp c=$capValue;\n";
         # }
         die("Cap $tokens[0] is connected to a bus, not allowed at this level\n");
   	}
   	else {
         print OUT "$tokens[0] $outInSeparator $powernet $signet c=$capValue;\n";
   	}
   	next;
   }
   
   if( /^[Vv]/ ) {
      s/'//g;
      @tokens = split /\s+/, $_;
      print OUT "$tokens[0] $outInSeparator ";
      for( $i = 1; $i <=$#tokens -1 ; $i++ ) {
         print OUT "$tokens[$i] ";
      }
      if( $tokens[$i] =~ /^([0-9.]+)[vV]$/ ) {
         $tokens[$i] = $1;
      }
      print OUT "v=$tokens[$i];\n";
      next;
   }
   
   if( /^[Rr]/ ) {
      # print "WARNING : Found a resistor. It will be considered to be of value 0\n";
      # print "        : Hence not accounted for delay.\n";
      # print "        : Also only one series resistor is allowed on any net connecting\n";
      # print "        : the output of a gate and the input of its fanout gate.\n";
   	s/'//g;
   	@tokens = split /\s+/, $_;
   	print OUT "$tokens[0] $outInSeparator ";
   	for( $i = 1; $i <=$#tokens -1 ; $i++ ) {
         print OUT "$tokens[$i] ";
   	}
   	if    ( $tokens[$i] =~ /^[0-9]+\.?[0-9]*$/ ) {
         if($tokens[$i] <= 0) {
            print "The resistance $tokens[0] should be positive.\n";
            print "It will be replaced in the optimizer by a small value -  1e-6Kohms.\n";
         }
         $tokens[$i] = $tokens[$i]/1000;
         print OUT "r=$tokens[$i];\n";
   	}
   	elsif ( $tokens[$i] =~ /^([0-9]+\.?[0-9]*)[Kk]*$/ ) {
   	   print OUT "r=$1;\n";
   	}
   	elsif ( $tokens[$i] =~ /^([0-9]+\.?[0-9]*)[M]*$/ ) {
   	   $resvalue = 0.001*$1;
   	   print OUT "r=$resvalue;\n";
   	}
   	else {
         print OUT "r=$tokens[$i];\n";
   	}
   	next;
   }
   
   if( /^\*input\s*:\s+(.+);/ ) {
      $stt = $1;
      if($stt =~ /\[(.*):(.*)\]/s) {
         if($1>$2) {
           $start_bus = $2;
           $end_bus = $1;
         }
         else {
            $start_bus = $1;
            $end_bus = $2;
         }
         for( $bus=$start_bus; $bus <= $end_bus; $bus++ ) {
            $newstt = $stt;
            $newstt =~ s/\[(.*):(.*)\]/[$bus]/g;
            print PI "$newstt;\n";
         }
      }
      else {
         print PI "$stt;\n";
      }
      next;
   }
   
   if( /^\*output\s*:\s+(.+);/ ) {
      $stt = $1;
      if($stt =~ /\[(.*):(.*)\]/s) {
         if( $1 > $2 ) {
            $start_bus = $2;
            $end_bus   = $1;
         }
         else {
            $start_bus = $1;
            $end_bus   = $2;
         }
         for( $bus=$start_bus; $bus <= $end_bus; $bus++ ) {
            $newstt = $stt;
            $newstt =~ s/\[(.*):(.*)\]/[$bus]/g;
            print PO "$newstt;\n";
         }
      }
      else { 
         print PO "$stt;\n";
      }
      next;
   }
   
   if( /^\*glbcnst/ ) {
      @tokens = split /\s*:\s*/, $_;
      print GLCNST "$tokens[1]";
      next;
   }
   
   if( /^\*localcnst/ ) {
      push( @ {$localConst{$subcktname}}, $_ );
      next;
   }
   
   if( /^\*\s*(AntiCorr)\s+([^\s]+)\s+([^\s]+)\s*;/ && $inSub == 1 ) {
      print "Recording $_";
      print " in subckt $subcktname\n";
      print GLCNST "$1 $subcktname $2 $3;\n";
      next;
   }
   if( /^\*\s*(AntiCorr)\s+([^\s]+)\s+([^\s]+)\s+([^\s]+)\s*;/ && !($inSub == 1) ) {
      print "Recording $_";
      print " in subckt $subcktname\n";
      print GLCNST "$1 $2 $3 $4;\n";
      next;
   }
   if( /^\*\s*(IntPrecharge)\s+(.+)\s*;/ ) {
      print "Recording $_";
      if($inSub == 1) {
         print GLCNST "$1 $subcktname $2;\n";
         print " in subckt $subcktname\n";
         next;
      }
      else {
         print GLCNST "$1 $2;\n";
         next;
      }
   }
   
   if( /^\*g_glbcnst/ ) {
      print "g_glbcnst is no longer supported- Ignoring\n";
      next;
      @tokens = split /\s*:\s*/, $_;
      if($tokens[1] =~ /width\s+<\s+(.*)/s) {    
         foreach $key (keys %hash_width) {
            if($hash_width{$key} == 1) {
               print GLCNST "$key < $1";
            }
         }
      }
      elsif($tokens[1] =~ /width\s+>\s+(.*)/s) {
         foreach $key (keys %hash_width) {
            if($hash_width{$key} == 1) {
               $inv_w = 1/$1;
               print GLCNST "$key^-1 < $inv_w;\n";
            }
         }
      }
      next;
   }
   
   if( /$global*/     ) { next; }
   if( /^\s*\.END\s+/ ) { next; }
   s/\s*$// ;
   s/$/;\n/ ; 
   print OUT $_;
}

if($inTopCell == 1) {
   print OUT "$ends\n\n";
}

close( IN  );
close( OUT );

print PI     ".ENDS\n\n";
print PO     ".ENDS\n\n";
print GLCNST ".ENDS\n\n";

close( PI     );
close( PO     );
close( GLCNST );

if( $optfileDefined == 1 ) {
   system("cat $filePrefix$pi $filePrefix$po $filePrefix.glc $optfile >> $outfile;
   rm -f $filePrefix$pi $filePrefix$po $filePrefix.glc ");	
}
else {
   print "Optimization file to be included later for -sol option \n";
   # print "Remember to have it for optim.pl -sol \n";
   # print "You can include the .OPTIMIZE section yourself in $outfile \n";
   print " \n";
   system("cat $filePrefix$pi $filePrefix$po $filePrefix.glc >> $outfile;
   rm -f $filePrefix$pi $filePrefix$po $filePrefix.glc ");
   # mv $outfile $infile");
}


################################################################################
################################################################################
sub isVddOrGnd {
   my $net = shift(@_);
   if( lc( $net ) eq "gnd" or lc( $net ) eq "vdd" or lc( $net ) eq "vss" ) {
      return 1;
   }
   else {
      return 0;
   }
}

