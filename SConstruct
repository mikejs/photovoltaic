# -*- mode: python; -*-
import os

AddOption('--c99',
          dest='use_c99',
          default=False,
          action='store_true',
          help='Compile with C99 (recommended for gcc)')

env = Environment(ENV=os.environ)
env.Append(LIBPATH=".")
env.Append(CFLAGS=" -pedantic -Wall -ggdb ")
conf = Configure(env)
libs = ["mongoc", "bson"]

if GetOption('use_c99'):
    env.Append(CFLAGS=" -std=c99 ")
else:
    if not conf.CheckType('int64_t'):
        if conf.CheckType('int64_t', '#include <stdint.h>\n'):
            conf.env.Append( CFLAGS=" -DMONGO_HAVE_STDINT " )
        elif conf.CheckType('int64_t', '#include <unistd.h>\n'):
            conf.env.Append( CFLAGS=" -DMONGO_HAVE_UNISTD " )
        elif conf.CheckType('__int64'):
            conf.env.Append( CFLAGS=" -DMONGO_USE__INT64 " )
        elif conf.CheckType('long long int'):
            conf.env.Append( CFLAGS=" -DMONGO_USE_LONG_LONG_INT " )
        else:
            print "*** what is your 64 bit int type? ****"
            Exit(1)

def checkLib( n ):
    if conf.CheckLib( n ):
        return True
    print( "Error: can't find library: " + str( n ) )
    Exit(-1)
    return False

for x in libs:
    checkLib( x )

env = conf.Finish()

files = ['main.c', 'oplog.c']

env.Program('photovoltaic', files, LIBS=libs)

env.Default(['photovoltaic'])
