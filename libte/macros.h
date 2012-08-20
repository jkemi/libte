/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef MACROS_H_
#define MACROS_H_

#ifdef _MSC_VER
  #ifdef BUILDING_DLL
    #define TE_EXPORT __declspec(dllexport)
  #else
    #define TE_EXPORT __declspec(dllimport)
  #endif
  #define TE_LOCAL
#else
  #if (__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
    #define TE_EXPORT __attribute__ ((visibility("default")))
    #define TE_LOCAL __attribute__ ((visibility("hidden")))
  #else
    #define TE_EXPORT
    #define TE_LOCAL
  #endif
#endif

#endif /* MACROS_H_ */
