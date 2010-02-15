# -*- mode: python; -*-
import os
import subprocess

AddOption('--c99',
          dest='use_c99',
          default=False,
          action='store_true',
          help='Compile with C99 (recommended for gcc)')

libs = ["mongoc", "bson", "curl"]

env = Environment(ENV=os.environ)
env.AppendUnique(LIBPATH=["."])
env.Append(CFLAGS=" -pedantic -Wall -ggdb ")
env.Append(CFLAGS=subprocess.Popen(
        ["xml2-config", "--cflags"], stdout=subprocess.PIPE).communicate()[0])

xml_libs = subprocess.Popen(['xml2-config', '--libs'],
                            stdout=subprocess.PIPE).communicate()[0].split()
env.AppendUnique(LIBPATH=[xml_libs[0][2:]])
for lib in xml_libs[1:]:
    libs.append(lib[2:])

conf = Configure(env)

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

files = ['main.c', 'schema.c', 'oplog.c', 'solr.c']

env.Program('photovoltaic', files, LIBS=libs)

env.Default(['photovoltaic'])
