#!/usr/bin/python
import sys, os, re, jpsy
################################################################################
# putSectionInOpt.py
################################################################################
#
# written    (in Perl)   18 OCT 2005 by Dinesh Patil 
# re-written (in Python) 26 JUN 2012 by Pete Stevenson
#
# This script takes in the name of the section and looks for that section in the input file  specified. 
# It then copies that section to the Optfile (typically _mod.sp). Also if the
# Optfile already has that section, it is overwritten. This helps to put in the activity 
# factors, TM gate info etc in the Optfile, in case we use it again and again to generate 
# these numbers.
#
# If the section appears more than once, it is copied multiple times, but if any section 
# instance is empty, it is not copied. All the occurances
# of a section are deleted from the Optfile. This feature is useful to copy subckts.
#
################################################################################
################################################################################
def DeleteSection( inpLines, section ):
   # delete the section from the Optfile and copy the rest in the dummy file.
   outLines = []
   inSection = False
   for line in inpLines:
      if re.search( '\s*\.' + section, line ):
         inSection = True
      if not inSection:
         outLines.append( line )
      if re.search( '\s*\.ENDS', line ):
         inSection = False
   return outLines

def AddSection( inpLines, section, includeFile ):
   includeLines = jpsy.ReadFileLines( includeFile )
   sectionLines = []
   foundSection = False
   inSection    = False
   for line in includeLines:
      if re.search( '\s*\.' + section, line ):
         inSection    = True
         foundSection = True
      if inSection:
         sectionLines.append( line )
      if re.search( '\s*\.ENDS', line ):
         inSection = False
   assert foundSection
   return inpLines + sectionLines

################################################################################
################################################################################
( path, prog ) = os.path.split( sys.argv[0] )

# Check for proper number of arguments and extract args
if len( sys.argv ) != 4:
   print 'Usage:', prog, 'Optfile FileToIncludeFrom SectionName'
   print 'EXAMPLE:', prog, 'adder_64mod.sp foo.sp POWER'
   sys.exit( -1 )

inputFile   = sys.argv[ 1 ]
includeFile = sys.argv[ 2 ]
sectionName = sys.argv[ 3 ]

lines = jpsy.ReadFileLines( inputFile )
lines = DeleteSection( lines, sectionName )
lines = AddSection( lines, sectionName, includeFile )
jpsy.WriteLinesToFile( lines, inputFile )
