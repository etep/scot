#!/usr/bin/python
import sys, os, re, math, time, shutil, jpsy
################################################################################
################################################################################
class Params:
   verbose = True
   nirsims = 8000
   EDTrade = True
   numInteriorPoints = 20
   interiorPointZeroFraction = 1e-6
   envname = 'SCOT_HOME_DIR'
   envtemp = 'SCOT_TEMP_DIR'
   def SetupTempPath( self ):
      theTime = time.localtime()
      yr   = '%04d' % ( theTime.tm_year )
      mon  = '%02d' % ( theTime.tm_mon  )
      day  = '%02d' % ( theTime.tm_mday )
      hr   = '%02d' % ( theTime.tm_hour )
      mt   = '%02d' % ( theTime.tm_min  )
      sec  = '%02d' % ( theTime.tm_sec  )
      date = '_'.join( [yr, mon, day] )
      tyme = '.'.join( [hr, mt,  sec] )
      path = '_'.join( [ 'scot', date, tyme ] )
      self.runpath = os.path.join( self.tempHome, path )
      os.makedirs( self.runpath )
      os.environ[ 'SCOT_TEMP_DIR_OPTIM_DOT_PY' ] = self.runpath
   
   def __init__( self ):
      
      if not ( self.envname in os.environ and self.envtemp in os.environ ):
         ( progpath, progname ) = os.path.split( sys.argv[0] )
         print 'Before using ' + progname + ', you need to set the environment variables "' + self.envname + '" and "' + self.envtemp + '"'
         print 'to point to the home directory for this project and to a suitable place for temporary files.'
         sys.exit( -1 )
      
      self.scotHome = os.environ[ self.envname ]
      self.tempHome = os.environ[ self.envtemp ]
      self.pysHome  = os.path.join( self.scotHome, 'pys'  )
      self.tekHome  = os.path.join( self.scotHome, 'tech' )
      self.txtHome  = os.path.join( self.scotHome, 'txt'  )
      
      self.optpy    = os.path.join( self.pysHome, 'optim.py' )
      self.glbfile  = os.path.join( self.tekHome, 'glb90n_par.sp' )
      self.datfile  = os.path.join( self.tekHome, 'Tech90.dat'  )
      self.prmfile  = os.path.join( self.tekHome, 'scmos90.prm' )
      self.cmdfile  = os.path.join( self.txtHome, 'optcmd.txt'  )
      
      self.ggpsolHome = os.path.join( self.scotHome, 'ggpsolexp', 'bin' )
      self.ggpsolbin  = os.path.join( self.ggpsolHome, 'ggpsolexp' )
      
      self.SetupTempPath()
   

################################################################################
################################################################################
def logspace( beg, end, numpoints ):
   numpoints += 2
   beg = float( beg )
   end = float( end )
   expbeg  = math.log( beg, 10.0 )
   expend  = math.log( end, 10.0 )
   logvec = []
   delta  = expend - expbeg
   assert delta > 0.0
   assert numpoints > 2
   step = delta / ( float( numpoints ) - 1.0 )
   for idx in range( numpoints ):
      logvec.append( 10.0**( expbeg + float(idx) * step ) )
   return logvec[ 1:-1 ]

################################################################################
################################################################################
def AddEDTradeConstraintsToHsp( hspfile ):
   lines = jpsy.ReadFileLines( hspfile )
   lines.append( '*glbcnst: OnlyFormulateProblem;'   )
   lines.append( '*glbcnst: E     < 99999999999999;' )
   lines.append( '*glbcnst: POMAX < 99999999999999;' )
   jpsy.WriteLinesToFile( lines, hspfile )

def FindOptimalValue( solFile, varname ):
   
   lines = jpsy.ReadFileLines( solFile )
   
   inoptvars = False
   for line in lines:
      if line.find( 'Inequality Constraints:'  ) == 0: break
      if inoptvars:
         toks = line.split()
         if len( toks ) == 2:
            name  = toks[0]
            value = float( toks[1] )
            if name == varname:
               return value
      if line.find( 'Optimal Variable Values:' ) == 0: inoptvars = True
   assert False

def FindOptimalEDPoint( solFile ):
   
   pomax  = None
   etotal = None
   lines  = jpsy.ReadFileLines( solFile )
   
   inoptvars = False
   for line in lines:
      if line.find( 'Inequality Constraints:'  ) == 0: break
      if inoptvars:
         toks = line.split()
         if len( toks ) == 2:
            name  = toks[0]
            value = float( toks[1] )
            if name == 'POMAX':   pomax  = value
            if name == 'E_TOTAL': etotal = value
      if line.find( 'Optimal Variable Values:' ) == 0: inoptvars = True
   assert pomax  is not None
   assert etotal is not None
   return ( etotal, pomax )

def FindMinDelay( params, origGpFile ):
   
   minDGPFile = jpsy.SwapFext( origGpFile, 'min.delay.gp' )
   
   lines = jpsy.ReadFileLines( origGpFile )
   assert lines[0].index( 'minimize' ) == 0
   lines[0] = 'minimize obj_epi_var;'
   lines.append( 'obj_epi_var_constraint : POMAX < obj_epi_var;' )
   
   jpsy.WriteLinesToFile( lines, minDGPFile )
   ggpsolcmd = ' '.join( [ params.ggpsolbin, '-d', minDGPFile ] )
   jpsy.SystemWrapper( ggpsolcmd )
   
   solFile  = jpsy.SwapFext( minDGPFile, 'out' )
   minDel   = FindOptimalValue( solFile, 'POMAX' )
   return minDel

def FindMinEnergy( params, origGpFile ):
   
   minEGPFile = jpsy.SwapFext( origGpFile, 'min.energy.gp' )
   
   lines = jpsy.ReadFileLines( origGpFile )
   assert lines[0].index( 'minimize' ) == 0
   lines[0] = 'minimize obj_epi_var;'
   lines.append( 'obj_epi_var_constraint : E_TOTAL < obj_epi_var;' )
   
   jpsy.WriteLinesToFile( lines, minEGPFile )
   ggpsolcmd = ' '.join( [ params.ggpsolbin, '-d', minEGPFile ] )
   jpsy.SystemWrapper( ggpsolcmd )
   
   solFile  = jpsy.SwapFext( minEGPFile, 'out' )
   minNrg   = FindOptimalValue( solFile, 'E_TOTAL' )
   return minNrg

def FindInteriorPoint( params, origGpFile, kidx, kval ):
   
   gpfile = jpsy.SwapFext( origGpFile, 'ed.%03d.gp' % kidx )
   
   lines = jpsy.ReadFileLines( origGpFile )
   assert lines[0].index( 'minimize' ) == 0
   lines[0] = 'minimize obj_epi_var;'
   lines.append( 'obj_epi_var_constraint : POMAX + %g * E_TOTAL < obj_epi_var;' % kval )
   print 'obj_epi_var_constraint : POMAX + %e * E_TOTAL < obj_epi_var;' % kval
      
   jpsy.WriteLinesToFile( lines, gpfile )
   ggpsolcmd = ' '.join( [ params.ggpsolbin, '-d', gpfile ] )
   jpsy.SystemWrapper( ggpsolcmd )
   
   solFile  = jpsy.SwapFext( gpfile, 'out' )
   edpoint  = FindOptimalEDPoint( solFile )
   return edpoint

def GetSanitizedGPFile( params ):
   oldGpfn = os.path.join( params.runpath, 'MINDDDET' )
   newGpfn = jpsy.SwapFext( oldGpfn, 'gp' )
   shutil.copy( oldGpfn, newGpfn )
   return newGpfn

def DoEDTradeOff( params ):
   gpfile = GetSanitizedGPFile( params )
   minDel = FindMinDelay( params, gpfile )
   minNrg = FindMinEnergy( params, gpfile )
   midpoint = minDel / minNrg
   beg  = midpoint * params.interiorPointZeroFraction
   end  = midpoint / params.interiorPointZeroFraction
   kvec = logspace( beg, end, params.numInteriorPoints )
   edpoints = []
   for kidx, kval in enumerate( kvec ):
      edpoint = FindInteriorPoint( params, gpfile, kidx, kval )
      edpoints.append( edpoint )
   print ''
   print ''
   print ''
   print 'minDelay  =', minDel
   print 'minEnergy =', minNrg
   for edpoint in edpoints:
      print edpoint
   print ''
   print ''
   print ''

################################################################################
################################################################################
params = Params()

hspfile = sys.argv[1]
sspfile = jpsy.SwapFext( hspfile, 'ssp' )
diofile = jpsy.SwapFext( hspfile, 'dio' )

# copy the hspice file into the tmp/run path
assert os.path.isfile( hspfile )
shutil.copy( hspfile, os.path.join( params.runpath, hspfile ) )

hspfile = os.path.join( params.runpath, hspfile )
sspfile = os.path.join( params.runpath, sspfile )
diofile = os.path.join( params.runpath, diofile )

assert os.path.isfile( hspfile )
if params.EDTrade:
   AddEDTradeConstraintsToHsp( hspfile )

( path, filename ) = os.path.split( hspfile )
if path != '': os.chdir( path )



hspCmd = ' '.join( [ params.optpy, '-hsp', hspfile, params.glbfile, sspfile ] )
irsCmd = ' '.join( [ params.optpy, '-irs', hspfile, sspfile, params.prmfile, str( params.nirsims ) ] )
sspCmd = ' '.join( [ params.optpy, '-ssp', sspfile, params.datfile, diofile ] )
solCmd = ' '.join( [ params.optpy, '-sol', diofile, sspfile, params.cmdfile ] )

jpsy.SystemWrapper( hspCmd, verbose = params.verbose, trial = False )
jpsy.SystemWrapper( irsCmd, verbose = params.verbose, trial = False )
jpsy.SystemWrapper( sspCmd, verbose = params.verbose, trial = False )
jpsy.SystemWrapper( solCmd, verbose = params.verbose, trial = False )

if params.EDTrade:
   DoEDTradeOff( params )





