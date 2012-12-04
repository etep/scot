#!/usr/bin/python
import sys
from spectreparse import JoinContinuedLines
from spectreparse import SpectreNetlistParser
from spectreparse import ReadFileLines
################################################################################
################################################################################
def RemoveVddGnd( ilist ):
    olist = []
    for p in ilist:
        if p.lower() in [ 'vdd', 'gnd', 'vss' ]: continue
        olist.append( p )
    return olist

def ConvertPortList( ilist ):
    olist = []
    for p in ilist:
        if p.lower() == 'vss': p = 'gnd'
        if p.lower().startswith( 'out' ):
            p = 'e_' + p
        olist.append( p )
    return olist

def ConvertPortName( p ):
    if p.lower().startswith( 'out' ):
        p = 'e_' + p
    return p

def ConvertInstance( i, scnames ):
    i.connections = ConvertPortList( i.connections )
    if i.mname in scnames:
        i.connections = RemoveVddGnd( i.connections )
    return i

def ConvertSubckt( sc, scnames ):
    pnames = []
    sc.portnames = ConvertPortList( sc.portnames )
    sc.portnames = RemoveVddGnd( sc.portnames )
    for iidx, instance in enumerate( sc.instantiations ):
        instance = ConvertInstance( instance, scnames )
        sc.instantiations[ iidx ] = instance
    return sc

def SCOTConvertComments( s ):
    if s.startswith( '*' ) and s.count( '//' ):
        toks = s.split( '//' )
        toks = [ t.lstrip().rstrip() for t in toks ]
        assert toks[0] == '*'
        text = '//'.join( toks[ 1: ] )
        toks = text.split()
        if toks[0] in [ 'input:', 'output:', 'glbcnst:' ]:
            t0 = '*' + toks[0]
            toks = [ t0 ] + toks[ 1: ]
            return ' '.join( toks )
    return s

def SCOTConvertMosfets( s ):
    if s.lower().startswith( 'm' ):
        toks = s.split( '=' )
        toks = [ t.lstrip().rstrip() for t in toks ]
        s = '='.join( toks )
        toks = s.split()
        for tidx, t in enumerate( toks ):
            if t.count( '=' ):
                subtoks = t.split( '=' )
                v = subtoks.pop()
                k = subtoks.pop()
                assert len( subtoks ) == 0
                if k == 'w':
                    k = 'W'
                try:
                    x = float( v )
                except:
                    v = "'(" + v + ")'"
                t = '='.join([ k, v ])
            toks[ tidx ] = t
        s = ' '.join( toks )
    return s

def SCOTConvertVSources( s ):
    if s.lower().startswith( 'v' ):
        return ' '.join([ '*', s ])
    return s

################################################################################
################################################################################
nlparser = SpectreNetlistParser()

lines = ReadFileLines( sys.argv[ 1 ] )
lines = JoinContinuedLines( lines )
s = '\n'.join( lines )

netlist = []
subckts = []

nlparser.Parse( s )

scnames = nlparser.SubCktNames()

print '* first line in hspice'

for s in nlparser.subckts:
    ConvertSubckt( s, scnames )

for i in nlparser.netlist:
    ConvertInstance( i, scnames )

lines = []
for item in nlparser.items:
    s = item.hspstring()
    lines.append( s )

lines = [ l.lower() for l in lines ]
s = '\n'.join( lines )
lines = s.split( '\n' )

for lidx, line in enumerate( lines ):
    line = SCOTConvertVSources( line )
    line = SCOTConvertComments( line )
    line = SCOTConvertMosfets( line )
    lines[ lidx ] = line

s = '\n'.join( lines )

print s
print ''
print '.GLOBAL gnd vdd'
print '.END'





