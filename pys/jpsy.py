import time, os, sys, pickle
################################################################################
################################################################################
def ReadFileLines( fn = None ):
   # opens a file
   # returns the contents as a list of lines in the file
   # trailing new-line chars are removed
   if fn is None:
      print 'Usage: ReadFileLines( fileName )'
      print 'returns the file contents as a list of lines with newline chars stripped'
      assert False
   FH = open( fn, "r" )
   lines = [ line.rstrip() for line in FH.readlines() ]
   FH.close()
   return lines

def WriteLinesToFile( lines = None, fn = None ):
   if lines is None or fn is None:
      print 'Usage: WriteLinesToFile( lines, fileName )'
      print 'joins the list of lines supplied with a newline char, writes them to the file specified'
      assert False
   FH = open( fn, "w" )
   FH.write( "\n".join( lines ) + "\n" )
   FH.close()

################################################################################
################################################################################
def DumpPickle( obj, fn ):
   FH = open( fn, 'w' )
   pickle.dump( obj, FH )
   FH.close()

def Unpickle( fn ):
   FH = open( fn, 'r' )
   x = pickle.load( FH )
   FH.close()
   return x

################################################################################
################################################################################
def SwapFext( filename, fext ):
   toks = filename.split( '.' )
   toks.pop()
   toks.append( fext )
   return '.'.join( toks )

################################################################################
################################################################################
def AddRedirectionToCmd( cmd, fn ):
  return " ".join([ cmd, "1>", fn, "2>&1" ])

def PrintTimeStamp():
  theTime = time.localtime()
  yr  = '%04d' % ( theTime.tm_year )
  mon = '%02d' % ( theTime.tm_mon  )
  day = '%02d' % ( theTime.tm_mday )
  hr  = '%02d' % ( theTime.tm_hour )
  mt  = '%02d' % ( theTime.tm_min  )
  sec = '%02d' % ( theTime.tm_sec  )
  print '...', '/'.join( [yr, mon, day] ), ':'.join( [hr, mt, sec] )
  sys.stdout.flush()

def RemoveWrapper( fn, verbose = False, trial = False ):
   if os.path.isfile( fn ):
      if verbose:
         PrintTimeStamp()
         print '... os.remove(', fn, ')'
         sys.stdout.flush()
      if not trial:
         os.remove( fn )

def SystemWrapperPipelined( cmd, verbose = False, trial = False ):
  if verbose:
    PrintTimeStamp()
    print '... os.popen(', cmd, ')'
    sys.stdout.flush()
  if not trial:
    FH = os.popen( cmd )
    output = [ line.rstrip() for line in FH.readlines() ]
    FH.close()
    return output
  return []

def SystemWrapper( cmd, verbose = False, trial = False ):
   dt = 0
   if verbose:
      PrintTimeStamp()
      print '... os.system(', cmd, ')'
      sys.stdout.flush()
   if trial:
      status = 0
   else: 
      t0 = time.time()
      status = os.system( cmd )
      t1 = time.time()
      dt = t1 - t0
   assert status == 0
   if verbose:
      toks = cmd.split()
      ( cmdPath, rawCmd ) = os.path.split( toks[0] )
      print '...', '%5.2f' % (dt) , 'seconds elapsed, finished:', ' '.join([ rawCmd, toks[1] ])
      sys.stdout.flush()
   return dt

################################################################################
################################################################################
def ForkLaunch( cmds, logs, maxFork = 1, verbose = True, trial = False ):
   # sanity:
   assert len( cmds ) == len( logs )
   #
   # setup:
   cmdslogs = zip( cmds, logs )
   numLaunched = 0
   #
   # fork & launch:
   for ( cmd, log ) in cmdslogs:
      pid = os.fork()
      cmd = AddRedirectionToCmd( cmd, log )
      if pid == 0:
         SystemWrapper( cmd, verbose = verbose, trial = trial )
         sys.exit( 0 )
      else:
         numLaunched += 1
         if numLaunched >= maxFork:
            os.waitpid( -1, 0 )
   while True:
      try:    os.waitpid( -1, 0 )
      except: break

