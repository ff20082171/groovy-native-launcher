//  A simple library for creating a native launcher for a java app
//
//  Copyright (c) 2006 Antti Karanta (Antti dot Karanta (at) iki dot fi) 
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
//  Author:  Antti Karanta (Antti dot Karanta (at) iki dot fi) 
//  $Revision$
//  $Date$


#if !defined( _JVMSTARTER_H_ )

#include <jni.h>

#if defined( __cplusplus )
  extern "C" {
#endif

#define _JVMSTARTER_H_

#if defined(_WIN32)

#  define JST_FILE_SEPARATOR "\\"
#  define JST_PATH_SEPARATOR ";"

#else

#  define JST_FILE_SEPARATOR "/"
#  define JST_PATH_SEPARATOR ":"

#endif

/** set to true at startup to print debug information about what the launcher is doing to stdout */
jboolean _jst_debug ;    
    
typedef enum { 
  /** a standalone parameter, such as -v */
  JST_SINGLE_PARAM,
  /** a parameter followed by some additional info, e.g. --endcoding UTF8 */
  JST_DOUBLE_PARAM,
  /** a parameter that its value attached, e.g. param name "-D", that could be given on command line as -Dfoo */
  JST_PREFIX_PARAM
} JstParamClass ;

typedef enum {
  JST_IGNORE       = 0,
  // both of the next two may be set at the same time, use & operator to find out
  JST_TO_JVM       = 1,
  JST_TO_LAUNCHEE  = 2, 
  JST_UNRECOGNIZED = 4
} JstInputParamHandling ;

typedef struct {
  char          *name ;
  JstParamClass type ;
  /** If != 0, the actual parameters followed by this one are passed on to the launchee. */
  unsigned char terminating ;
  JstInputParamHandling handling ;
} JstParamInfo ;

// this is to be taken into use after some other refactorings
typedef struct {
  char *param ;

  // for twin (double) parameters, only the first will contain a pointer to the corresponding param info
  JstParamInfo* paramDefinition ;
  // char *paramName ;
  
  // this is needed so that the cygwin conversions need to be performed only once
  // This is the actual value passed in to the jvm or launchee program. E.g. the two (connected) parameters
  // --classpath /usr/local:/home/antti might have values "--classpath" and "C:\programs\cygwin\usr\local;C:\programs\cygwin\home\antti"
  // respectively
  char *value ;
  JstInputParamHandling handling ; 
    
} JstActualParam ; 

// TODO: remove this completely - there should be a char* to append stuff to classpath
typedef enum {
// classpath handling constants
/** if neiyher of these first two is given, CLASSPATH value is always appended to jvm classpath */
  JST_IGNORE_GLOBAL_CP = 1,
/** global CLASSPATH is appended to jvm startup classpath only if -cp / --classpath is not given */
  JST_IGNORE_GLOBAL_CP_IF_PARAM_GIVEN = 2,
/** this can be given w/ the following */
  JST_CP_PARAM_TO_APP = 4,
/** pass the given cp param to jvm startup classpath */
  JST_CP_PARAM_TO_JVM = 8
} ClasspathHandling ;

/** The strategy to use when selecting between using client and server jvm *if* no explicit -client/-server param. If explicit param
 * is given, then that type of jvm is used, period.
 * Bitwise meaning:
 *  001 -> allow using client jvm
 *  010 -> allow using server jvm
 *  100 -> prefer client over server
 * If you're just passing this as a param, you need not worry about the bitwise meaning, it's just an implementation detail. */
typedef enum { 
  JST_CLIENT_FIRST     = 7,
  JST_SERVER_FIRST     = 3,
  JST_TRY_CLIENT_ONLY  = 1,
  JST_TRY_SERVER_ONLY  = 2
} JVMSelectStrategy ;

/** javahome handling constants.
 * In order of precedence: 
 *  - programmatically given jh 
 *  - jh user parameter 
 *  - JAVA_HOME env var 
 *  - java home deduced from java executable found on path 
 *  - win registry (ms windows only) / "/System/Library/Frameworks/JavaVM.framework" (os-x only)
 */
typedef enum {
  JST_USE_ONLY_GIVEN_JAVA_HOME = 0,
/** If given java_home == null, try to look it up from JAVA_HOME environment variable. */
  JST_ALLOW_JH_ENV_VAR_LOOKUP = 1,
/** Allow giving java home as -jh / --javahome parameter. The precedence is 
 * -jh command line parameter, java_home argument and then JAVA_HOME env var (if allowed). */
  JST_ALLOW_JH_PARAMETER = 2,
/** Check windows registry when looking for java home. */
  JST_ALLOW_REGISTRY_LOOKUP = 4,
  /** Check if java home can be found by inspecting where java executable is located on the path. */ 
  JST_ALLOW_PATH_LOOKUP     = 8
} JavaHomeHandling ;

// FIXME - delete this, (almost) groovy specific
// these can be or:d together w/ |
typedef enum {
  JST_IGNORE_TOOLS_JAR       = 0,
  // JST_TOOLS_JAR_TO_CLASSPATH = 1,
  JST_TOOLS_JAR_TO_SYSPROP   = 2
} ToolsJarHandling ;

 /** Note that if you tell that -cp / --classpath and/or -jh / --javahome params are handled automatically. 
  * If you do not want the user to be able to affect 
  * javahome, specify these two as double params and their processing is up to you. 
  */
typedef struct {
  /** May be null. */
  char* javaHome ;
  /** what kind of jvm to use. */
  JVMSelectStrategy jvmSelectStrategy ;
  // TODO: ditch this, the called may preprocess them. Provide a func to transform a space separated string into jvm options
  /** The name of the env var where to take extra jvm params from. May be NULL. */
  char* javaOptsEnvVar ;
  /** what to do about tools.jar */
  ToolsJarHandling toolsJarHandling ;
  /** See the constants above. */
  JavaHomeHandling javahomeHandling ; 
  /** See the constants above. */
  ClasspathHandling classpathHandling ; 
  /** The arguments the user gave. Usually just give argv + 1 */
  char** arguments ; 
  /** The arguments the user gave. Usually just give argv - 1 */
  int numArguments ;
  /** extra params to the jvm (in addition to those extracted from arguments above). */
  JavaVMOption* jvmOptions ;
  int numJvmOptions; 
  /** parameters to the java class' main method. These are put first before the command line args. */
  char** extraProgramOptions ;
  char*  mainClassName ;
  /** Defaults to "main" */
  char*  mainMethodName ;
  /** The directories from which add all jars from to the startup classpath. NULL terminates the list. */
  char** jarDirs ;
  char** jars ;
  /** parameterInfos is an array containing info about all the possible program params. The terminating JstParamInfo has NULL for name. */
  JstParamInfo* paramInfos ;
  /** terminatingSuffixes contains the suffixes that, if matched, indicate that the matching param and all the rest of the params 
   * are launcheeParams, e.g. {".groovy", ".gy", NULL}. 
   * The significance of this is marginal, as any param w/ no preceding "-" is terminating. So, this is only significant
   * if the terminating option has "-" as prefix, but is not one of the enumerated options. Usually this would be
   * a file name associated w/ the app, e.g. "-foobar.groovy". As file names do not usually begin w/ "-" this is rather unimportant. */
  char** terminatingSuffixes ;
} JavaLauncherOptions ;


#if defined( _WIN32 )
// to have type DWORD in the func signature below we need this header
//#include "Windows.h"
// DWORD is defined as unsigned long, so we'll just use that
/** Prints an error message for the given windows error code. */
void jst_printWinError( unsigned long errcode ) ;

#endif

int jst_launchJavaApp( JavaLauncherOptions* options ) ;

int jst_fileExists( const char* fileName ) ;

/** Returns 1 if the given fileName points to a dir, 0 if a file. If the given file does not exist, the behavior is undefined, most likely
 * a crash. */
int jst_isDir( const char* fileName ) ;

/** Figures out the path to the parent dir of the given path (which may be a file or a dir). Modifies the argument so
 * that it points to the parent dir. Returns null (but does not modify the given string) if the given dir is the root dir.
 * For files, the dir containing the file is the parent dir. 
 * Note that the given path must be absolute and canonical for this function to work ( no e.g. ..'s ) */
char* jst_pathToParentDir( char* path ) ;

/** Returns a NULL terminated string array containing the names of files in the given dir. The returned string array is dyn 
 * allocated. It and all the contained strings are freed by freeing the returned pointer.
 * @param fileNamePrefix return only files whose name begins with this prefix. May be NULL or empty string.
 * @param fileNameSuffix return only files whose name ends with this suffix.   May be NULL or empty string. 
 *        Suffix here means the file type identifier part, e.g. in "foo.jar" -> ".jar" */
char** jst_getFileNames( char* dirName, char* fileNamePrefix, char* fileNameSuffix ) ;

/** Creates a string that represents a file (or dir) name, i.e. all the elements are
 * ensured to contain a file separator between them. 
 * Usage: give as many strings as you like and a NULL as the last param to terminate. */
char* jst_createFileName( const char* root, ... ) ;


/** Returns the path to the directory where the current executable lives, excluding the last path separator, e.g.
 * c:\programs\groovy\bin or /usr/loca/groovy/bin 
 * Note that this means that if the program lives in the root dir in *nix, "" is returned.
 * Do NOT modify the returned string, make a copy. 
 * Returns NULL on error. */
char* jst_getExecutableHome() ;

/** Returns the index of the given str in the given str array, -1 if not found.  
 * Modifies args, numargs and checkUpto if removeIfFound == true */
int jst_contains(char** args, int* numargs, const char* option, const jboolean removeIfFound);

/** may return argc if none of the presented params are "terminating", i.e. indicate that it and all the rest of the params
 * go to the launchee. 
 * @param the terminating JstParamInfo has NULL for name. */
int jst_findFirstLauncheeParamIndex( char** argv, int argc, char** terminatingSuffixes, JstParamInfo* paramInfos ) ;

/** returns an array of JstActualParam, the last one of which contains NULL for field param. 
 * All the memory allocated can be freed by freeing the returned pointer.
 * Return NULL on error. */
JstActualParam* jst_processInputParameters( char** args, int numArgs, JstParamInfo *paramInfos, char** terminatingSuffixes ) ;

/** returns null if not found. For prefix params, returns the value w/out the prefix.
 * paramType is double or prefix.
 * In case of double param w/ no value, error out param is set to true. */
char* jst_valueOfParam(char** args, int* numargs, int* checkUpto, const char* option, const JstParamClass paramType, const jboolean removeIfFound, jboolean* error);

/** returns -1 if not found */
int jst_indexOfParam( char** args, int numargs, char* paramToSearch) ;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// dynamic memory handling
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** Appends the given strings to target. size param tells the current size of target (target must have been
 * dynamically allocated, i.e. not from stack). If necessary, target is reallocated into a bigger space. 
 * Returns the possibly new location of target, and modifies the size inout parameter accordingly. 
 * If target is NULL, it is allocated w/ the given size (or bigger if given size does not fit all the given strings). 
 * In case target is NULL and you are not interested how big the buffer became, you can give NULL as size. */
char* jst_append( char* target, size_t* size, ... ) ; 

/** Concatenates the strings in the given null terminated str array to a single string, which must be freed by the caller. Returns null on error. */
char* jst_concatenateStrArray( char** nullTerminatedStringArray ) ;

/** If array is NULL, a new one will be created, size arlen. The given array will be reallocated if there is not enough space.
 * The newly allocated memory (in both cases) contains all 0s. If NULL is given as the item, zeroes are added at the given array position. */
void* jst_appendArrayItem( void* array, int index, size_t* arlen, void* item, int item_size_in_bytes ) ;

/** Appends the given pointer to the given null terminated pointer array.
 * given pointer to array may point to NULL, in which case a new array is created. 
 * Returns NULL on error. 
 * @param item if NULL, the given array is not modified. */
void** jst_appendPointer( void*** pointerToNullTerminatedPointerArray, size_t* arrSize, void* item ) ;

int jst_pointerArrayLen( void** nullTerminatedPointerArray ) ;

/** Returns the given item, NULL if the item was not in the array. */
void* jst_removePointer( void** nullTerminatedPointerArray, void* itemToBeRemoved ) ;

/** returns 0 if the given item was not in the given array. pointer pointed to is set to NULL. */
int jst_removeAndFreePointer( void** nullTerminatedPointerArray, void** pointerToItemToBeRemoved ) ;


/** Given a null terminated string array, makes a dynamically allocated copy of it that can be freed using a single call to free. Useful for constructing
 * string arrays returned to caller who must free them. In technical detail, the returned mem block first contains all the char*, the termiting NULL and
 * then all the strings one after another. Note that whoever uses the char** does not need to know this mem layout. */
char** jst_packStringArray( char** nullTerminatedStringArray ) ;

typedef enum { PREFIX_SEARCH, SUFFIX_SEARCH, EXACT_SEARCH } SearchMode;

/** The first param may be NULL, it is considered an empty array. */
jboolean jst_arrayContainsString( char** nullTerminatedArray, const char* searchString, SearchMode mode ) ; 

/** These wrap the corresponding memory allocation routines. The only difference is that these print an error message if
 * the call fails. */
void* jst_malloc( size_t size ) ;
void* jst_calloc( size_t nelem, size_t elsize ) ;
void* jst_realloc( void* ptr, size_t size ) ;
char* jst_strdup( const char* s ) ;


#define jst_free( x ) free( x ) ; x = NULL

/** Frees all the pointers in the given array, the array itself and sets the reference to NULL */
void jst_freeAll( void*** pointerToNullTerminatedPointerArray ) ;


/** As appendArrayItem, but specifically for jvm options. 
 * @param extraInfo JavaVMOption.extraInfo. See jni.h or jni documentation (JavaVMOption is defined in jni.h). */
JavaVMOption* appendJvmOption( JavaVMOption* opts, int index, size_t* optsSize, char* optStr, void* extraInfo ) ;

#if defined( __cplusplus )
  } // end extern "C"
#endif

#endif // ifndef _JVMSTARTER_H_
