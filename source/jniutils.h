//  A library for easy creation of a native launcher for Java applications.
//
//  Copyright (c) 2006-2009 Antti Karanta (Antti dot Karanta (at) hornankuusi dot fi)
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in
//  compliance with the License. You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software distributed under the License is
//  distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
//  implied. See the License for the specific language governing permissions and limitations under the
//  License.
//
//  Author:  Antti Karanta (Antti dot Karanta (at) hornankuusi dot fi)

#include "jni.h"

#ifndef JST_JNIUTILS_H_
#define JST_JNIUTILS_H_

#if defined( __cplusplus )
  extern "C" {
#endif

/** To be called when there is a pending exception that is the result of some
 * irrecoverable error in this startup program. Clears the exception and prints its description. */
void clearException( JNIEnv* env ) ;

jclass getJavaStringClass( JNIEnv* env ) ;

#if defined( __cplusplus )
  } // end extern "C"
#endif


#endif /* JST_JNIUTILS_H_ */
