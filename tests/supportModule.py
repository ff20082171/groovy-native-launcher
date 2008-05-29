# -*- mode:python coding:utf-8 -*-
# jedit: :mode=python:

#  Groovy -- A native launcher for Groovy
#
#  Copyright © 2008 Russel Winder
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

import os
import re
import tempfile

#  The relative path to the executable.
executablePath = None

#  The value of environment['PLATFORM'], so the values are posix, darwin, sunos, cygwin, win32.
architecture = None

#  Temporary file names generated by Python can contain characters that are not permitted as Java class
#  names.  This function generates temporary files that are compatible.

def javaNameCompatibleTemporaryFile ( ) :
    while True :
        file = tempfile.NamedTemporaryFile ( )
        if re.compile ( '[-]' ).search ( file.name ) == None : break
        file.close ( )
    return file

#  Execute a Groovy activity returning a tuple of the return value and the output.  On Ubuntu and Cygwin the
#  return value is an amalgam of signal and return code.  Fortunately we know that the signal is in the low
#  byt and the return value in the next byte.

def executeCommand ( command ) :
    process = os.popen ( executablePath + ' ' + command )
    output = process.read ( ).strip ( )
    returnCode = process.close ( )
    if returnCode != None :
        #  Different platforms do different things with the return value.  This sucks.
        if platform in [ 'posix' , 'darwin' , 'sunos' , 'cygwin' ] :
            returnCode >>= 8
    return ( returnCode , output )
  
if __name__ == '__main__' :
    print 'Run tests using command "scons test".'
