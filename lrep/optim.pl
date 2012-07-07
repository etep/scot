#!/usr/bin/perl -w


#########################################################################
# optim.pl
#########################################################################

# Written 8/16/2005 by Dinesh Patil 

# This script runs the various optimization tasks on a bunch of appropriate 
# files using the various options. It calls many other perl scripts from itself.
# 

#########################################################################
# Load Libraries
#
#
#########################################################################
use File::Basename;
use Getopt::Long;


# Check for proper number of arguments and extract args
$opt_sp  = 0;
$opt_mod = 0;
$opt_sol = 0;
$opt_irsim = 0;
$opt_bsu   = 0;
$opt_bsp   = 0;
$opt_beldo = 0;
$opt_ggp   = 0;
$opt_ggpx  = 0;
$opt_psvo  = 0;
$opt_gpIter  = 0;
$opt_psn     = 0;
$opt_extract = 0;
$opt_paf  = 0;
$opt_pdf  = 0;
$opt_area = 0;
$transmission = "TRANSMISSION";

($prog) = fileparse($0);

Usage() if ( ! GetOptions(
           'sp'      => \$opt_sp,
           'mod'     => \$opt_mod, 
           'sol'     => \$opt_sol,
           'ggpx'    => \$opt_ggpx,
           'irsim'   => \$opt_irsim,
           'bsu'     => \$opt_bsu,
           'bsp'     => \$opt_bsp,
           'beldo'   => \$opt_beldo,
           'ggp'     => \$opt_ggp,
           'psvo'    => \$opt_psvo,
           'gpIter'  => \$opt_gpIter,
           'psn'     => \$opt_psn,
           'extract' => \$opt_extract,
           'paf'     => \$opt_paf,
           'pdf'     => \$opt_pdf,
           'area'    => \$opt_area) );

$opt_sum = $opt_sp + $opt_mod +  $opt_sol + $opt_ggpx + $opt_irsim + $opt_bsu +$opt_bsp + $opt_beldo + $opt_ggp + $opt_psvo + $opt_gpIter + $opt_psn + $opt_extract + $opt_paf + $opt_pdf + $opt_area;

Usage() if ($opt_sum == 0);

if( $opt_sum > 1 ) {
   die("Only one option allowed per script call\n");
}

if    ( $opt_sp    == 1 ) {
   if( $#ARGV != 2 ) {
      # $#ARGV is the number of command line arguments minus 1
      print STDERR "Usage: $prog -sp heirarchical_spice_infile glb_param_file output_file\n";
      print STDERR "EXAMPLE: $prog -sp foo.sp glb_param.sp foo_mod.sp\n";
      print STDERR "The output file is used by the optimizer\n";
      exit;
   }
   $infile  = shift( @ARGV );
   $glbfile = shift( @ARGV );
   $outfile = shift( @ARGV );
   $tmpfile = $infile."_tmp_opt_sp";
   system("./gate_sub_flat.pl $infile $tmpfile mainlist");
   system("./sue_spice.pl $glbfile $tmpfile $outfile");
   system("./endsendshack.pl $outfile");
   # system("rm -f $tmpfile");
}
elsif ( $opt_mod   == 1 ) {
   if( $#ARGV != 2 ) {
      # $0 is the script name
      print STDERR "Usage: $prog -mod modified_spice_file dio_data_file output_dio_file\n";
      print STDERR "EXAMPLE: $prog -mod foo_mod.sp ST9015var.dat foo_mod.dio\n";
      print STDERR "foo_mod.dio contains the delay/leakage models for CCCs in foo_mod.sp\n";
      exit;
   }
   $spfile = shift(@ARGV);
   $diofile = shift(@ARGV);
   $outfile = shift(@ARGV);
   system("diogen -d $spfile $diofile $outfile");
   system("putSectionInOpt.pl $spfile $spfile.txgate $transmission");
}
elsif ( $opt_irsim == 1 ) {
   if( $#ARGV < 3 || $#ARGV >=  5 )  { 
      print STDERR "Usage: $prog -irsim spice_file(with reasonble sizes) modified_spice_file irsim_tech_file #_irsim_runs  [optional]<.cmd file for IRSIM runs>\n"; # $0 is the script name
      print STDERR "EXAMPLE: $prog -irsim foo.sp(back anotated) foo_mod.sp scmos30.prm 1000 <test_foo.cmd>(if you have it)\n";
      print STDERR "The activity(duty) factors will be in file foo_mod.sp.power(duty) and also be included in foo_mod.sp\n";
      print STDERR "If the user provides the .cmd file, s/he should also provide the #iterations\n";
      exit;
   }
   $spfile = shift(@ARGV);
   $modfile = shift(@ARGV);
   $techfile = shift(@ARGV);
   $numRuns = shift(@ARGV);
   $actfile = $modfile.".power";
   $dutyfile = $modfile.".duty";
   if( $#ARGV == 0 ) {
      $cmdfile = shift(@ARGV);
      system("diogen -p $spfile $modfile $techfile $numRuns $cmdfile");
   }
   else {
      system("diogen -p $spfile $modfile $techfile $numRuns");
   }
   print "Warning: If the modified spice file already has a power/duty section in it, they WOULD be overwritten\n";
   # print "This may result in error while solving it, Please delete those sections before hand\n";
   system("putActFactInOpt.pl $modfile  $actfile");
   system("putDutyFactInOpt.pl $modfile $dutyfile");
}
elsif ( $opt_paf   == 1 ) {
   if( $#ARGV != 1 )  { 
      print STDERR "Usage: $prog -paf modified_spice_file file_with_activity_factors\n"; 
      print STDERR "EXAMPLE: $prog -paf foo_mod.sp foo_mod.sp.power\n";
      exit;
   } 
   $modfile = shift(@ARGV);
   $actfile = shift(@ARGV);
   system( "putActFactInOpt.pl $modfile $actfile" );
}
elsif ( $opt_pdf   == 1 ) {
   if( $#ARGV != 1 ) { 
      print STDERR "Usage: $prog -pdf modified_spice_file file_with_duty_factors\n"; 
      print STDERR "EXAMPLE: $prog -pdf foo_mod.sp foo_mod.sp.duty\n";
      exit;
   } 
   $modfile = shift(@ARGV);
   $dutyfile = shift(@ARGV);
   system("putDutyFactInOpt.pl $modfile $dutyfile");
}
elsif ( $opt_sol   == 1 ) {
   if( $#ARGV != 2 ) {
      # $0 is the script name
      print STDERR "Usage: $prog -sol modified_spice_file delay_model_file opt_spec_file\n";
      print STDERR "EXAMPLE: $prog -sol foo_mod.sp foo_mod.dio foo.opt\n";
      print STDERR "The results are generated as per the tasks and file names specified in opt_spec_file\n";
      exit;
   }
   $modfile = shift(@ARGV);
   $dumfile = $modfile."_optimize";
   $diofile = shift(@ARGV);
   $optfile = shift(@ARGV);
   system("cp $modfile $dumfile");
   system("cat $optfile >> $dumfile");
   system("ciropt $dumfile $diofile");
   # system("rm -f $dumfile");
}
elsif ( $opt_bsu   == 1 ) {
   if ($#ARGV != 1)  {
      # $0 is the script name
      print STDERR "Usage: $prog -bsu top_cell.sue data_file.out\n";
      print STDERR "EXAMPLE: $prog -bsu foo.sue fooDDET.out\n";
      print STDERR "The resulting sue file(along with the hierarchy) will be in top_cell_opt.sue\n";
      exit;
   }
   $suefile = shift(@ARGV);
   $datafile = shift(@ARGV);
   system( "back_annotate.pl $suefile $datafile" );
}
elsif ( $opt_bsp   == 1 ) {
   if( $#ARGV != 3 )  { 
      # $0 is the script name
      print STDERR "Usage: $prog -bsp data_file original_spice_file modified_spice_file output_spice_file\n";
      print STDERR "EXAMPLE: $prog -bsp fooDDET.out foo.sp foo_mod.sp foo_opt.sp \n";
      print STDERR "foo_opt.sp contains the back annotated spice netlist\n";
      exit;
   }
   $datafile = shift(@ARGV);
   $spfile = shift(@ARGV);
   $modfile = shift(@ARGV);
   $outfile = shift(@ARGV);
   system("opt2spice.pl $datafile $spfile $modfile $outfile");
}
elsif ( $opt_beldo == 1 ) {
   if( $#ARGV < 3 )  { 
      print STDERR "Usage: $prog -beldo data_file original_spice_file modified_spice_file output_eldo_file <NMOS name> <PMOS name> (both optional)\n"; # $0 is the script name
      print STDERR "EXAMPLE: $prog -beldo fooDDET.out foo.sp foo_mod.sp foo_opt.eldo nhvt psvt\n";
      print STDERR "foo_opt.eldo is the output file. Default NMOS and PMOS names are \"nsvt\" and \"psvt\" resp.\n";
      exit;
   }
   $datafile = shift(@ARGV);
   $spfile = shift(@ARGV);
   $modfile = shift(@ARGV);
   $eldofile = shift(@ARGV);
   $tmpfile = $modfile."_tmp_opt_sp";
   if    ( $#ARGV == 1 ) {
      $nName = shift(@ARGV);
      $pName = shift(@ARGV);
   }
   elsif ( $#ARGV == 0 ) {
      $nName = shift(@ARGV);
      $pName = "psvt";
   }
   else {
      $nName = "nsvt";
      $pName = "psvt";
   }
   system("opt2spice.pl $datafile $spfile $modfile $tmpfile");
   system("spiceToEldo.pl $tmpfile $eldofile $nName $pName");
   system("rm -f $tmpfile");
}
elsif ( $opt_ggp   == 1 ) {
   if( $#ARGV != 0 ) { 
      print STDERR "Usage: $prog -ggp ggp_input_file\n"; # $0 is the script name
      print STDERR "EXAMPLE: $prog -ggp fooDDET\n";
      print STDERR "The solution will be in fooDDET.out\n";
      exit;
   }
   $ggpfile = shift(@ARGV);
   system("ggpsol -d $ggpfile");
}
elsif ( $opt_ggpx  == 1 ) {
   if( $#ARGV != 0 ) { 
      # $0 is the script name
      print STDERR "Usage: $prog -ggpx ggp_input_file\n"; 
      print STDERR "EXAMPLE: $prog -ggpx fooDDET\n";
      print STDERR "The solution will be in fooDDET.out\n";
      exit;
   }
   $ggpfile = shift(@ARGV);
   system("ggpsolexp -d $ggpfile");
}
elsif ( $opt_psn   == 1 ) {
   if( $#ARGV != 0 ) { 
      print STDERR "Usage: $prog -psn top_cell.sue\n"; # $0 is the script name
      print STDERR "EXAMPLE: $prog -psn foo32.sue\n";
      print STDERR "The same sue hierarchy results, now with names.\n";
      print STDERR "Names are given only to cells in the same directory as the top_cell.\n";
      print STDERR "Note that this does not name buses or wires that are not\n";
      print STDERR "attached to a name-net label.\n";
      exit;
   }
   $suefile = shift(@ARGV);
   system("putSueNames.pl $suefile");
}
elsif ( $opt_extract == 1 ) {
   if( $#ARGV != 1 ) { 
      print STDERR "Usage: $prog -extract SolutionFileListAndVariableListFile matlabFileName\n"; # $0 is the script name
      print STDERR "EXAMPLE: $prog -extract foo.data matlab.dat\n";
      print STDERR "The matlab.dat file will contain the info about what column is which variable.\n";
      exit;
   }
   $datafile = shift(@ARGV);
   $matfile = shift(@ARGV);
   system("ExtractValues.pl $datafile $matfile");
}
elsif ( $opt_psvo    == 1 ) {
   
   if( $#ARGV < 2 ) { 
      # $0 is the script name
      print STDERR "Usage: $prog -psvo ggpsol_input_file solution_file Vthmargin(absolute) <old_VthVal file> (if it exists)\n"; 
      print STDERR "EXAMPLE: $prog -psvo fooDDET fooDDETprev.out 0.01 <fooDDETprev_VthVal\n";
      print STDERR "The output ggpsol file : fooDDET_snapVth   The file with snapped Vth values: fooDDET_VthVal or the fooDDETprev_VthVal file if specified\n";
      exit;
   }
   $ggpfile = shift( @ARGV );
   $resfile = shift( @ARGV );
   $margin  = shift( @ARGV );
   if( $#ARGV == 3 ) {
      $oldVthFile = shift(@ARGV);
      system( "PutSnappedVthValues.pl $ggpfile $resfile $margin $oldVthFile" );
   }
   else {
      system( "PutSnappedVthValues.pl $ggpfile $resfile $margin" );
   }
}
elsif ( $opt_gpIter  == 1 ) {
     if ($#ARGV < 4)  { 
         print STDERR << "EOP"; 
         Usage: $prog -gpIter ggpsol_input_template_file New_ggpfilename_containing_<VAL_MARK> constraint_name log_file list_of_values
         EXAMPLE: $prog -gpIter fooDDET fooEVAL_MARKDET total_energy run.log 800 1000 1200 1500 2000 ....

         In the above command for each energy, a new
         ggpsol input file will be created. For example,
         for 800, the file create will be fooE800DET
         which is (VAL_MARK replaced by 800). It is 
         then solved using ggpsol
         Also note that the numbers will replace the 
         the RHS of the constraint specified

         
EOP
         
         exit;
     }
     $templatefile = shift(@ARGV);
     $newfile = shift(@ARGV);
     $constraint = shift(@ARGV);
     $logfile = shift(@ARGV);
     $stt = " ";
     for ($i=0;$i<=$#ARGV;$i++) {
          $stt = $stt."$ARGV[$i] ";
     }
     system("runDelAreaPowIter.pl $templatefile $newfile $constraint $logfile nosnap $stt");
}
elsif ( $opt_area    == 1 ) {
   if( $#ARGV != 0 ) { 
      # $0 is the script name
      print STDERR "Usage: $prog -area irsimFile\n";
      print STDERR "EXAMPLE: $prog -area foo.sim\n";
      print STDERR "The script adds the total gate cap and the total wire cap.\n";
      exit;
   }
   $simfile = shift(@ARGV);
   system("area_estimate.pl $simfile");  
}
else {
   print "ERROR: No options given\n";
}

################################################################################
################################################################################
sub Usage {
print << "EOM";
Usage: $prog [option] respective_fileNames
Run the various optimization commands on the circuit netlist. 
Options and the corresponding files needed (in order) are:

  -sp  : convert a typical spice file into the 
          modified_spice_file needed by the 
          optimizer.
          
  -mod : obtain the delay and leakage models for 
          the CCCs in the netlist. The netlist_file 
          is also modified in the process to record 
          any transmission gates in the 
          netlist.
  
  -psn : Name all the blocks in the SUE hierarchy. 
          Also name all the nets which have the name-net
          hook attached to them. Nets that are buses
          cannot be named. The names help to track mistakes
          and block names are a must for back-annotation in SUE.
          
  -sol : optimize the netlist using the generated 
          models and the optimization and analysis 
          tasks specified by user.
          The files produced as a result are
          according to the names given in opt_file.

  -irsim : run IRSIM (provided IRSIM is installed) 
          on the netlist to find out the 
          activity factors, and include them in the 
          optimization file.
          If the user is providing the command file
          for IRSIM commands, then the number of runs
          should be entered correctly by the user.

  -paf   : put the activity factors in the mod-file from 
         the already available activity factors file. Used
         when the modified spice file gets over-written or
         otherwise modified and the power section is deleted.

  -pdf   : like paf, put duty factors into the modified file
         from a file containing a list of duty factors.

  -bsu : back annotates the results in the sue file
          and produces a new sue netlist with optimized
          sizes.
        
  -bsp : back annotate the results in spice file. The 
          file used by the optimizer is used.

  -beldo : same as -bsp except that the spice_file
          is then converted in to an eldo compatible 
          spice file.

  -ggp : solve the GGP problem using ggpsol and MOSEK.

  -ggpx: same as -ggp option except that the solution algrithm
          is exponential and is about an order of magnitude faster 
          than the -ggp option for large circuits. However, it does 
          not provide sensitivity information for constraints as it 
          does solve the dual problem.
          
  -psvo : Take Vth values from the solution file, snap 
          them to discrete values within a specified 
          margin and substitute them back into the 
          ggpsol input file to make a new problem for 
          iterative optimization. Also record the snapped
          Vth values in a separate file.
          
  -gpIter : Change one constraint and generate multiple solutions
           to obtain a tradeoff curve.   

  -extract : Given a file containing a list of solution file names and 
            variable names, go thru these solution files and gather the
            optimal values of the specified variables. Dump the values into
            the specified file in a MATLAB loadable format.

  -area   : Get the total gate width, the layout area and the total wire cap
            of your design from the sim file.
        
EOM
  exit;
#  -sp  original_spice_infile modified_spice_file glb_param_file 
#        :  convert a typical spice file into the 
#          modified_spice_file needed by the 
#          optimizer, using the global params 
#          for a given technology node.
#
#  -mod  modified_spice_file dio_data_file dio_model_file
#        : obtain the delay and leakage models for 
#          the CCCs in the modified_spice_file
#          using the basic dio models to get 
#          output_dio_file. The modified_spice_file 
#          is also modified in the process if 
#          there are transmission gates in the 
#          netlist.
#          
#  -sol  modifiled_spice_file dio_model_file opt_file
#        : Optimize the netlist using the models in 
#          dio_model_file and the optimization
#          and analysis tasks specified in opt_file.
#          The files produced as a result are
#          according to the names given in opt_file.
#          
#  -irsim original_spice_file(with reasonable sizes) modified_spice_file irsim_tech_file number_of_irsim_runs  [optional]<.cmd file for IRSIM runs> 
#        : runs IRSIM (provided IRSIM is installed) 
#          on the netlist to find out the 
#          activity factors, which are then recorded
#          in the file named modified_spice_file.power.
#          Also modified_spice_file is changed to include
#          the activity factors obtained from above. 
#          If the user is providing the command file
#          for IRSIM commands, then the number of runs
#          should be entered correctly by the user.
#          
#  -bsu  top_cell.sue data_file.out 
#        : back annotates the results in top_cell.sue
#          and produces a new file top_cell_opt.sue
#          that can be viewed in SUE.
#        
#  -bsp  solution_file original_spice_file modified_spice_file spice_out_file
#        : back annotate the results in the solution_file
#          in spice file to produce a spice_out_file that
#          can be used for simulation and verification. The 
#          original_spice_file is needed only for the 
#          header info (if any is present).
#
#  -beldo solution_file original_spice_file modified_spice_file spice_out_file <NMOS_name> <PMOS_name>
#        : This is same as -bsp except that the spice_file
#          is then converted in to an eldo compatible 
#          spice file. The NMOS and PMOS names to be
#          used are also specified (in that order).
#          They are optional and "nsvt" and "psvt" are
#          used in case nothing is specified.
#
#EOM
#  exit;
}


