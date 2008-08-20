/*
 * macros.h
 *
 *  Created on: Aug 17, 2008
 *      Author: jakob
 */

#ifndef MACROS_H_
#define MACROS_H_

#ifdef _MSC_VER
  #ifdef BUILDING_DLL
    #define DLLEXPORT __declspec(dllexport)
  #else
    #define DLLEXPORT __declspec(dllimport)
  #endif
  #define DLLLOCAL
#else
  #ifdef HAVE_GCCVISIBILITYPATCH
    #define DLLEXPORT __attribute__ ((visibility("default")))
    #define DLLLOCAL __attribute__ ((visibility("hidden")))
  #else
    #define DLLEXPORT
    #define DLLLOCAL
  #endif
#endif

#ifdef __cplusplus
#	define CDECLS_BEGIN	extern "C" {
#	define CDECLS_END	}
#else
#	define CDECLS_BEGIN
#	define CDECLS_END
#endif

#endif /* MACROS_H_ */
