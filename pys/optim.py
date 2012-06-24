#!/usr/bin/python
import sys, os, re, time, shutil, jpsy, argparse
################################################################################
################################################################################
class Params:
   verbose = True
   envname = 'SCOT_HOME_DIR'
   def __init__( self ):
      
      if not self.envname in os.environ:
         print 'Before using ' + sys.argv[0] + ', you need to set the environment variable "' + self.envname + '"'
         print 'to point to the home directory for this project.'
         sys.exit( -1 )
      
      self.scotHome = os.environ[ self.envname ]
      self.perlHome = os.path.join( self.scotHome, 'lrep' )
      
      self.gsfPerl = os.path.join( self.perlHome, 'gate_sub_flat.pl'    )
      self.suePerl = os.path.join( self.perlHome, 'sue_spice.pl'        )
      self.eehPerl = os.path.join( self.perlHome, 'endsendshack.pl'     )
      self.pafPerl = os.path.join( self.perlHome, 'putActFactInOpt.pl'  )
      self.pdfPerl = os.path.join( self.perlHome, 'putDutyFactInOpt.pl' )
      self.psiPerl = os.path.join( self.perlHome, 'putSectionInOpt.pl'  )
      
      self.diogenBin = os.path.join( self.scotHome, 'diogen', 'bin', 'diogen' )
      self.ciroptBin = os.path.join( self.scotHome, 'ciropt', 'bin', 'ciropt' )
   

################################################################################
################################################################################
def DashHspiceOption( args, params ):
   #
   # setup the filenames:
   hspfile = args.hspice[0]
   glbfile = args.hspice[1]
   outfile = args.hspice[2]
   rawfile = '.'.join( hspfile.split( '.' )[ :-1 ] )
   tmpfile = '.'.join([ rawfile, 'tmp', 'opt', 'sp' ])
   #
   # setup the command strings:
   # 1. run gate_sub_flat.pl
   # 2. run sue_spice.pl
   # 3. run endsendshack.pl
   gsfCmd = ' '.join( [ params.gsfPerl, hspfile, tmpfile, 'mainlist' ] )
   sueCmd = ' '.join( [ params.suePerl, glbfile, tmpfile, outfile    ] )
   eehCmd = ' '.join( [ params.eehPerl, outfile ] )
   #
   # execute the commands
   jpsy.SystemWrapper( gsfCmd, verbose = params.verbose, trial = False )
   jpsy.SystemWrapper( sueCmd, verbose = params.verbose, trial = False )
   jpsy.SystemWrapper( eehCmd, verbose = params.verbose, trial = False )

def DashIrsimOption( args, params ):
   #
   # setup the filenames:
   hspfile  = args.irsim[0]
   sspfile  = args.irsim[1]
   tekfile  = args.irsim[2]
   numruns  = int( args.irsim[3] )
   actfile  = '.'.join( [ sspfile, 'power' ] )
   dutyfile = '.'.join( [ sspfile, 'duty'  ] )
   rawfile = '.'.join( hspfile.split( '.' )[ :-1 ] )
   tmpfile = '.'.join([ rawfile, 'tmp', 'opt', 'sp' ])
   #
   # setup the command strings:
   # 1. run diogen
   # 2. run putActFactInOpt.pl
   # 3. run putDutyFactInOpt.pl
   dioCmd = ' '.join( [ params.diogenBin, '-p', hspfile, sspfile, tekfile, str( numruns ) ] )
   pafCmd = ' '.join( [ params.pafPerl, sspfile, actfile  ] )
   pdfCmd = ' '.join( [ params.pdfPerl, sspfile, dutyfile ] )
   #
   # call the commands:
   jpsy.SystemWrapper( dioCmd, verbose = params.verbose, trial = False )
   jpsy.SystemWrapper( pafCmd, verbose = params.verbose, trial = False )
   jpsy.SystemWrapper( pdfCmd, verbose = params.verbose, trial = False )

def DashScotSpiceOption( args, params ):
   #
   # setup the filenames:
   sspfile = args.scotspice[0]
   tekfile = args.scotspice[1]
   outfile = args.scotspice[2]
   txgfile = '.'.join( [ sspfile, 'txgate' ] )
   #
   # setup the commands:
   # 1. run diogen
   # 2. run put section in (putSectionInOpt.pl)
   dioCmd = ' '.join( [ params.diogenBin, '-d', sspfile, tekfile, outfile ] )
   psiCmd = ' '.join( [ params.psiPerl, sspfile, txgfile, 'TRANSMISSION' ] )
   #
   # call the commands:
   jpsy.SystemWrapper( dioCmd, verbose = params.verbose, trial = False )
   jpsy.SystemWrapper( psiCmd, verbose = params.verbose, trial = False )

def DashSolveOption( args, params ):
   #
   # setup the filenames:
   sspfile = args.solve[0]
   tekfile = args.solve[1]
   cmdfile = args.solve[2]
   cirfile = '.'.join( [ sspfile, 'ciropt' ] )
   #
   shutil.copy( sspfile, cirfile )
   #
   # setup the commands:
   # 1. cat
   # 2. ciropt
   catCmd = ' '.join( [ 'cat', cmdfile, '>>', tekfile ] )
   optCmd = ' '.join( [ params.ciroptBin, tekfile, cirfile ] )
   #
   # call the commands:
   jpsy.SystemWrapper( catCmd, verbose = params.verbose, trial = False )
   jpsy.SystemWrapper( optCmd, verbose = params.verbose, trial = False )

################################################################################
################################################################################
params = Params()
parser = argparse.ArgumentParser( description = 'The Stanford Circuit Optimization Tool (SCOT)' )

parser.add_argument( '-hsp', '--hspice',    nargs = 3, metavar = ( '<hsp file>', '<glb file>', '<out file>' ) )
parser.add_argument( '-ssp', '--scotspice', nargs = 3, metavar = ( '<ssp file>', '<tek file>', '<out file>' ) )
parser.add_argument( '-sol', '--solve',     nargs = 3, metavar = ( '<ssp file>', '<opt file>', '<cmd file>' ) )
parser.add_argument( '-irs', '--irsim',     nargs = 4, metavar = ( '<hsp file>', '<ssp file>', '<prm file>', '<num runs>' ) )

args = parser.parse_args()

if args.hspice is not None:
   DashHspiceOption( args, params )

if args.irsim is not None:
   DashIrsimOption( args, params )

if args.scotspice is not None:
   DashScotSpiceOption( args, params )

if args.solve is not None:
   DashSolveOption( args, params )
   











