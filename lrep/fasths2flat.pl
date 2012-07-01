#!/usr/bin/perl -w

# $Id: fasths2flat,v 1.1 2004/03/05 03:39:52 ddpatil Exp $

require 5.0;
use File::Basename;
use Getopt::Long;

$opt_r = 0;
$opt_lbl = 0;
$opt_su = 0;
$opt_t = 0;
$opt_h = 0;
$opt_v = 0;
($prog) = fileparse($0);


{

	Usage() 
	  if ( !GetOptions("h", "t", "r", "lbl", "su", "v") 
		|| $opt_h || $#ARGV != 0);

	$opt_su = 1 unless ($opt_lbl);

	$fileIn = $ARGV[0];

	$basename = fileparse($ARGV[0], '\..*');
	

	die "$prog: nonexistant input file $fileIn\n" 
		unless ( -f $fileIn );

	# $cmd = "spnet  -shortnames -width 5000 -emf -flat -noendcard -exp_notation $fileIn 1> $basename.flog 2>&1";
	$fileOut = $ENV{ "SCOT_TEMP_DIR_OPTIM_DOT_PY" } . "/" . "$basename.f";
	$cmd = "../pys/spnet.py $fileIn 1> $fileOut 2>&1";
	print STDERR "$prog: running\n\t$cmd\n" unless ( $opt_v );
	$cmdresult = system( $cmd );
	if( $cmdresult != 0 ) { die; }
	fsp2sim("$basename");

	unless ( $opt_r ) {
		# unlink("$basename.f");
		unlink("$basename.fal");
		unlink("$basename.fmap");
		unlink("$basename.spemap");
		unlink("$basename.spnmap");
		unlink("$basename.flog");
	}
}

sub Usage {
print << "EOM";
Usage: $prog [options] file.spi
Translate a spice file to a sim file for use in gemini. Options are:
	-r   : Do not remove intermediate files
	-t   : Do not translate the node names
	-lbl : Output LBL format (default is SU)
	-h   : This msg
EOM
	exit;
}



sub fsp2sim {
	my($tmpFileI) = @_;
	my($l, %tt, @ll);
	my($scale) = 1e-6;

	#die "$prog: Could not open $tmpFileI.fal\n" 
	#	unless open(FPI, "$tmpFileI.fal");

	#print STDERR "$prog: reading translation table in $tmpFileI.fal..\n"
	#	unless ( $opt_v );

  $tt{ "foo" } = "bar"; # dummy translation table
	# while ( <FPI> ) {
	# 	chomp;
	# 	@_ = split;
  # 
	# 	if ( $#_ < 2  || $_[0] ne "=") {
	# 	  next;
	# 	}
	# 	$tt{$_[1]} = $_[2];
	# }
	# close(FPI);

    $fileOut = $ENV{ "SCOT_TEMP_DIR_OPTIM_DOT_PY" } . "/" . "$tmpFileI.f";
	die "$prog: Could not open $fileOut\n" 
		unless open(FPI, "$fileOut" );

	print STDERR "$prog: dumping sim netlist...\n"
		unless ( $opt_v );
  SCAN:
	while ( <FPI> ) {
		chomp;
		@_ = split;
		next SCAN if ( $#_ < 0  );
		if ( $_[0] =~ /^m/ )  {
			print  Fet(\%tt, $scale,  @_);
		}
		elsif ( $_[0] =~ /^c/ ) {
			print  Cap(  \%tt, @_ );
		}
		elsif ( $_[0] =~ /^\.opt/ && $#_ >= 1 ) {
			@ll = split(/=/, $_[1]);
			if ( $#ll >= 1 ) {
				$scale = $ll[1] * 1e8 ;
			}
			printf "| units: %d tech: scmos format: %s\n",
			$scale, $opt_lbl ? "LBL": "SU" ;
		}
		elsif ( $_[0] =~ /^v/ ) {
			my($n1, $n2) = ( $_[1], $_[2]);

			unless ( $opt_t ) {
				$n1 = $tt{$n1} if ( exists $tt{$n1} );
				$n2 = $tt{$n2} if ( exists $tt{$n2} );
			}

			print "= $n1 $n2\n";
		}
	}
}



sub Fet {
	my ($tt, $scale, $name, $d, $g, $s, $b, $t, @args ) = @_;
	my ($l, $w, $as, $ad, $ps, $pd, $m, $geo) = ( 0, 0, 0, 0, 0, 0, 1, 0);
	my ($i, $var, $val);
	my ($r);
	
	for ($i=0 ; $i <= $#args; $i++) {
		$var = 0; 
		if ( $args[$i] =~ /^[a-z]/ ) {
			( $var, $val ) = getVar($i, \@args);
			#print "var=$var val=$val\n";
			print STDERR "Warning: Invalid mosfet argument $args[$i]\n" 
				if ( $val == -1 );
		}
		$l   = $val if ( $var eq "l"   );
		$w   = $val if ( $var eq "w"   );
		$as  = $val if ( $var eq "as"  );
		$ps  = $val if ( $var eq "ps"  );
		$ad  = $val if ( $var eq "ad"  );
		$pd  = $val if ( $var eq "pd"  );
		$geo = $val if ( $var eq "geo" );
		$m   = $val if ( $var eq "m"   );
	}

	unless ( $opt_t ) {
		$g = $$tt{$g} if ( exists $$tt{$g} );
		$d = $$tt{$d} if ( exists $$tt{$d} );
		$s = $$tt{$s} if ( exists $$tt{$s} );
		$b = $$tt{$b} if ( exists $$tt{$b} );
	}

	$t = "nmos" if ( $t eq "lnmos" );
	if ( $opt_lbl ) {
	   $r = sprintf "%s %s %s %s %s %d %d 0 0\n", substr($t, 0, 1),  $g, $d, $s, $b, $l, $w*$m;
	}
	else {
	   $r = sprintf "%s %s %s %s %.2f %.2f 0 0 g=G_%d,S_%s s=A_%d,P_%d d=A_%d,P_%d\n", substr($t, 0, 1),  $g, $d, $s, $l, $w*$m, $geo, $b, $as, $ps, $ad, $pd;
	}

	return $r;
}




#
# getVar($n,$l) -
# 	get a foo=num variable from line ref $@l at index $n
# 	return tuple with values or (0,-1) if parsing fails
sub getVar {
	my($n, $l) = (@_);
	my(@v, $val) = (0, -1);

	@v = split('=', @$l[$n], 2);
	if ( $#v ) {
		$val=$v[1] if ( $#v == 1 ) ;
		$val=@$l[$n+1] if ( substr(@$l[$n], -1, 1) eq "=" ) ;
	} elsif ( @$l[$n+1] eq "=" ) {
		$val = @$l[$n+2];
	} 
	if ( $val =~ /^[\$\*]/ ||  $val eq "") {
		$v[0] = 0;
		$val = -1;
	}
	return ($v[0], $val);
}


sub Cap {
	my ($tt, $name, $n1, $n2, $val) = @_;
	my($v);
	
	$v = capVal($val);

	$n1 = $$tt{$n1} if ( exists $$tt{$n1} );
	$n2 = $$tt{$n2} if ( exists $$tt{$n2} );
	return sprintf  "C %s %s %.2f\n", $n1, $n2, $val*1e15;
}


#
# capVal($c) -
#	return the value of a cap in fF
#
sub capVal {
	my($s) = (@_);
	my($sufx, $prfx) ;

	$s =~ /(\d*\.*\d*)(\D{2})/;
	$sufx = $2;
	$prfx = $1;
	return $prfx*1   if  ( $sufx =~ /^[fF][fF]*/ );
	return $prfx*1e3 if  ( $sufx eq /^[pP][fF]*/ );
	return $prfx*1e9 if  ( $sufx eq /^[uU][fF]*/ );
	return -1;
}
