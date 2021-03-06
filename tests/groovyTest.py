# -*- mode:python; coding:utf-8; -*-
# jedit: :mode=python:

#  Groovy -- A native launcher for Groovy
#
#  Copyright © 2008-9 Russel Winder
#
#  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in
#  compliance with the License. You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software distributed under the License is
#  distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
#  implied. See the License for the specific language governing permissions and limitations under the
#  License.

#  This tests realized in this Python script originated from the ones in the launcher_test.rb script by
#  Antti Karanta.

#  The directory in which the current executable is stored is calculated by SCons and includes the compiler
#  in use.  This means this test script is not entirely standalone but must be initiated by SCons.

import os
import re
import unittest
import shutil
import platform

import supportModule

from types import *

#  Class containing all the core tests.

class GroovyTestCase ( unittest.TestCase ) :

    def groovyExecutionTest ( self , command , expectedOutput = '' , expectedReturnCode = None , extraMessage = None , prefixCommand = '' ) :
        '''Execute a Groovy command and then test that the return code is right and the output is right.'''
        ( returnCode , output ) = supportModule.executeCommand ( command , prefixCommand )
        #print "return code ", returnCode
        #print "output ", output
        self.assertEqual ( returnCode , expectedReturnCode )
        if type ( expectedOutput ) == type ( re.compile ( 'a' ) ) :
            assert expectedOutput.search ( output ) != None , 'Failed to match ' +  ( extraMessage if extraMessage else output + ' against ' + str ( expectedOutput ) )
        elif type( expectedOutput ) == LambdaType :
            assert expectedOutput ( output ) , 'failed ' + str ( expectedOutput )
        else :
            self.assertEqual ( output , expectedOutput )

    def testVersion ( self ) :
        pattern = 'Groovy Version: .* JVM: '
        self.groovyExecutionTest ( '-v' , re.compile ( pattern ) , None , pattern )

    def testPassingJVMParameter ( self ) :
        # The exact amount of memory the JVM reserves on requesting -Xmx300m seems to vary by platform, but it seems to be always within 10% of that requested.
        self.groovyExecutionTest ( '-Xmx300m -e "println Runtime.runtime.maxMemory ( )"' ,  lambda x : abs ( int ( x ) - 300000000 ) < 30000000 )

    def testServerVM ( self ) :
        self.groovyExecutionTest ( '-server -e "println System.getProperty ( \'java.vm.name\' )"' , re.compile( 'server vm' , re.IGNORECASE ) , prefixCommand = "LD_LIBRARY_PATH='/usr/jdk/latest/jre/lib/sparc/server'" if supportModule.platform == 'sunos' else '' )

    def testClientVM ( self ) :
        #  It seems that there are no 64-bit client JVMs, only server JVMs for 64-bit.  Certainly this is
        #  true for the Solaris 10 Sparc, Russel's Ubuntu laptop and the Codehaus Bamboo test machine.  For
        #  the moment hack the test so that it works: os.environ['Width'] should be either 32 or 64.
        #  platform.uname ( )[4] is the architecture selector.
        self.groovyExecutionTest ( '-e "println System.getProperty ( \'java.vm.name\' )"' , re.compile(
            'server vm' if ( os.environ['Width'] == '64' ) or ( platform.uname ( )[4] == 'x86_64' ) else 'client vm' , re.IGNORECASE ) )

    def testExitStatus ( self ) :
        self.groovyExecutionTest ( '-e "System.exit ( 123 )"' , '' , 123 )

    def testLaunchingScript ( self ) :
        #  There is a weird problem with using filenames on MSYS, assume the same is true for Windwoes.
        tmpFile = file ( 'flobadob' , 'w+' ) if supportModule.platform == 'win32' else supportModule.javaNameCompatibleTemporaryFile ( )
        self.launchScriptTest ( tmpFile.name , tmpFile )
        if supportModule.platform == 'win32' : os.remove ( tmpFile.name )

    def launchScriptTest ( self , filename , theFile , extraMessage = None ) :
        theFile.write ( 'println \'hello \' + args[ 0 ]\n' )
        theFile.flush( )
        self.groovyExecutionTest ( filename + ' world' , 'hello world' )
        theFile.close ( )

#  Class for all the Cygwin tests.

class CygwinGroovyTestCase ( GroovyTestCase ) :
    
    def testPathUnconverted ( self ) :
        tmpFile = supportModule.javaNameCompatibleTemporaryFile ( )
        self.launchScriptTest ( tmpFile.name , tmpFile )

    def testPathConvertedBackslash ( self ) :
        tmpFile = supportModule.javaNameCompatibleTemporaryFile ( )
        filename = os.popen ( 'cygpath -w ' + tmpFile.name ).read ( ).strip ( ).replace ( '\\' , '\\\\' )
        self.launchScriptTest ( filename , tmpFile )

    def testPathConvertedForwardSlash ( self ) :
        tmpFile = supportModule.javaNameCompatibleTemporaryFile ( )
        filename = os.popen ( 'cygpath -w ' + tmpFile.name ).read ( ).strip ( ).replace ( '\\' , '/' )
        self.launchScriptTest ( filename , tmpFile )

    def testClasspathConversion ( self ) :
        aDirectory = '/tmp'
        bDirectory = aDirectory + '/foo'
        if os.path.exists ( bDirectory ) and not os.path.isdir ( bDirectory ) : os.remove ( bDirectory )
        if not os.path.isdir ( bDirectory ) : os.mkdir ( bDirectory )
        aFile = file ( aDirectory + '/A.groovy' , 'w' )
        aFile.write ( 'class A { def getB ( ) { return new B ( ) } }' )
        aFile.flush ( )
        bFile = file ( bDirectory + '/B.groovy' , 'w' )
        bFile.write ( 'class B { def sayHello ( ) { println( "hello there" ) } }' )
        bFile.flush ( )
        self.groovyExecutionTest ( '--classpath ' + aDirectory + ':' + bDirectory + ' -e "new A ( ).b.sayHello ( )"' , 'hello there' )
        os.remove ( aFile.name )
        os.remove ( bFile.name )
        #os.rmdir (bDirectory )
        shutil.rmtree( bDirectory, True )

#  The entry point for SCons to use.

def runTests ( path , architecture ) :
    return supportModule.runTests ( path , architecture , CygwinGroovyTestCase if architecture == 'cygwin' else GroovyTestCase )

if __name__ == '__main__' :
    print 'Run tests using command "scons test".'
