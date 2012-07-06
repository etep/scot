#!/usr/bin/python
import sys, os, re, argparse, jpsy, hspy
################################################################################
# putSectionInOpt.py
################################################################################
#
# written    (in Perl)   05 MAR 2004 by Dinesh Patil 
# re-written (in Python) 01 JUL 2012 by Pete Stevenson
#
# netlist flattening for irsim
#
################################################################################
################################################################################
class Params:
   verbose  = True
   suformat = True
   removeir = True
   transnodenames = False
   envscot = 'SCOT_HOME_DIR'
   envtemp = 'SCOT_TEMP_DIR_OPTIM_DOT_PY'
   def __init__( self ):
      ( progpath, prog ) = os.path.split( sys.argv[0] )
      self.prog = prog
      if not ( self.envscot in os.environ and self.envtemp in os.environ ):
         print 'Before using ' + prog + ', you need to set the environment variables:', self.envscot, 'and', self.envtemp
         print 'to point to the proper directories for this project.'
         sys.exit( -1 )
      
      self.scotHome = os.environ[ self.envscot ]
      self.tempHome = os.environ[ self.envtemp ]
      self.pysHome  = os.path.join( self.scotHome, 'pys'  )
   

################################################################################
################################################################################
def Usage( prog ):
   print 'Usage:', prog, '[-t] [-r] [-l] [-h] -f <file_name>'
   print 'Translate a spice file to a sim file for use in gemini. Options are:'
   print '   -r or --noremove    : Do not remove intermediate files'
   print '   -t or --nostranslate: Do not translate the node names'
   print '   -l or --lblformat   : Output LBL format (default is SU)'
   print '   -q or --quiet       : Less verbose output'
   print '   -h or --help        : This message'

################################################################################
################################################################################
def FillMissingFetParams( params ):
   if 'l' not in params: params[ 'l' ] = 0
   if 'w' not in params: params[ 'w' ] = 0
   if 'm' not in params: params[ 'm' ] = 1
   if 'geo' not in params: params[ 'geo' ] = 0
   if 'as'  not in params: params[ 'as'  ] = 0
   if 'ad'  not in params: params[ 'ad'  ] = 0
   if 'ps'  not in params: params[ 'ps'  ] = 0
   if 'pd'  not in params: params[ 'pd'  ] = 0
   return params

def SanitizeCapVal( v ):
   v = v.lower()
   suffix = ''
   if len( v ) > 1:
      suffix = v[ -2: ]
   if suffix in [ 'pf', 'nf', 'uf', 'ff' ]:
      v = v[ :-2 ]
   v = float( v )
   #
   # note: assumed units are fF
   if suffix == 'pf': v *= 1e3
   if suffix == 'nf': v *= 1e6
   if suffix == 'uf': v *= 1e9
   return str( 1e15 * v )

def Cap( params, tt, scale, line ):
   toks = line.split()
   farads = toks.pop()
   farads = SanitizeCapVal( farads )
   node1  = toks.pop()
   node0  = toks.pop()
   #
   if params.nodetranslate:
      node0 = tt[ node0 ]
      node1 = tt[ node1 ]
   cstr = 'C %s %s %s' % ( node0, node1, farads )
   return cstr

def Fet( params, tt, scale, line ):
   # -- TODO -- remove -- print 'line =', line
   ( line, mosparams ) = hspy.ExtractParameters( line )
   toks = line.split()
   # -- TODO -- remove -- print 'line =', line
   # -- TODO -- remove -- print mosparams
   # -- TODO -- remove -- assert False
   mosparams = FillMissingFetParams( mosparams )
   mtype = toks.pop()
   b     = toks.pop()
   s     = toks.pop()
   g     = toks.pop()
   d     = toks.pop()
   name  = toks.pop()
   try:
      assert len( toks ) == 0
   except:
      print 'toks =', toks
      print 'line =', line
      raise   
   if params.transnodenames:
      g = tt[ g ]
      d = tt[ d ]
      s = tt[ s ]
      b = tt[ b ]
   if mtype == 'lnmos': mtype = 'nmos'
   l = float( mosparams[ 'l' ] )
   m = float( mosparams[ 'm' ] )
   w = float( mosparams[ 'w' ] )
   lstr = str( l )
   wstr = str( m*w )
   if params.suformat:
      irstr = '%s %s %s %s %s %s 0 0 g=G_%s,S_%s s=A_%s,P_%s d=A_%s,P_%s' % ( mtype[0], g, d, s, lstr, wstr, mosparams[ 'geo' ], b, mosparams[ 'as' ], mosparams[ 'ps' ], mosparams[ 'ad' ], mosparams[ 'pd' ] )
   else:
      irstr = '%s %s %s %s %s %s %s' % ( mtype[0], g, d, s, b, lstr, wstr )
   return irstr

def Fsp2Sim( params, ifn ):
   inpLines = []
   outLines = []
   scale = 1e-6
   # -- perl -- variable names were shortened then translated back -- #print STDERR "$prog: reading translation table in $tmpFileI.fal..\n"
   # -- perl -- variable names were shortened then translated back -- #  unless ( $opt_v );
   # -- translation table note:
   # -- ... read translation table from file into dictionary
   # -- ... at the moment, using an empty dictionary, i.e. a dummy translation table
   tt = {}
   
   ofn = jpsy.SwapFext( ifn, 'f' )
   ofn = os.path.join( params.tempHome, ofn )
   
   if params.verbose:
      print params.prog + ': dumping sim netlist...'
   
   inpLines = jpsy.ReadFileLines( ifn )
   
   # extract the .opt scale setting (if it exists)
   for line in inpLines:
      if len( line ) > 4:
         if line[ 0:4 ] == '.opt':
            line = hspy.PackEqualsSigns( line )
            toks = line.split()
            if len( toks[1] ) > 4:
               if toks[1] == 'scale':
                  subtoks = toks[1].split( '=' )
                  assert len( subtoks ) == 2
                  scale = 1e8 * float( subtoks[1] )
   
   if params.suformat: fmtstr = 'SU'
   else:               fmtstr = 'LBL'
   outLines.append( '| units: %.6f tech: scmos format %s' % ( 1e8*scale, fmtstr ) )
   # -- TODO -- remove -- outLines.append( '= vdd Vdd' )
   # -- TODO -- remove -- outLines.append( '= gnd Gnd' )
   
   for line in inpLines:
      try:
         if line == '': continue
         toks = line.split()
         if   toks[0][0] == 'm':
            outLines.append( Fet( params, tt, scale, line ) )
         elif toks[0][0] == 'c':
            outLines.append( Cap( params, tt, scale, line ) )
         elif toks[0][0] == 'v':
            n0 = toks[1]
            n1 = toks[2]
            if params.transnodenames:
               n0 = tt[ n0 ]
               n1 = tt[ n1 ]
            outLines.append( '= %s %s' % ( n0, n1 ) )
      except:
         print '------------------------------------------------------------------------------------------------------------'
         print 'line =', line
         print '------------------------------------------------------------------------------------------------------------'
         raise
   return outLines

################################################################################
################################################################################
params = Params()
parser = argparse.ArgumentParser( description = 'The Stanford Circuit Optimization Tool (SCOT): fasths2flat.py' )

#parser.add_argument( '-h', '--help',        )
parser.add_argument( '-t', '--notranslate'  ) # do not translate node names; deprecated (for now)
parser.add_argument( '-r', '--noremove'     ) # do not remove the intermediate files
parser.add_argument( '-l', '--lblformat'    ) # output lbl format (default is su)
parser.add_argument( '-q', '--quiet'        ) # less verbosity
parser.add_argument( '-i', '--infilename',  nargs = 1 ) # -- TODO -- make this a required argument
parser.add_argument( '-o', '--outfilename', nargs = 1 ) # -- TODO -- make this a required argument

args = parser.parse_args()

# -- TODO -- add correct hook to show the usage message on -h or --help
#
#if args.help is not None:
#   Usage()
#   sys.exit( -1 )
assert params.suformat
assert params.verbose
assert args.infilename  is not None
assert args.outfilename is not None

ifn = args.infilename[0]
ofn = args.outfilename[0]
assert os.path.isfile( ifn )

if args.lblformat is not None:
   params.suformat = False

if args.quiet is not None:
   assert False
   params.verbose = False

( filepath, basename ) = os.path.split( ifn )


tfn = os.path.join( params.tempHome, basename + '.spnet.flattened' )
cmd = ' '.join( [ 'spnet.py', '-i', ifn, '-o', tfn ] )
cmd = os.path.join( params.pysHome, cmd )

jpsy.SystemWrapper( cmd, verbose = params.verbose, trial = False )

lines = Fsp2Sim( params, tfn )
jpsy.WriteLinesToFile( lines, ofn )

# -- TODO -- cleanup?? -- perl -- 	unless ( $opt_r ) {
# -- TODO -- cleanup?? -- perl -- 		# unlink("$basename.f");
# -- TODO -- cleanup?? -- perl -- 		unlink("$basename.fal");
# -- TODO -- cleanup?? -- perl -- 		unlink("$basename.fmap");
# -- TODO -- cleanup?? -- perl -- 		unlink("$basename.spemap");
# -- TODO -- cleanup?? -- perl -- 		unlink("$basename.spnmap");
# -- TODO -- cleanup?? -- perl -- 		unlink("$basename.flog");
# -- TODO -- cleanup?? -- perl -- 	}
# -- TODO -- cleanup?? -- perl -- }

