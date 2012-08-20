CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (LIBTE)
# The version number.
set (LIBTE_VERSION_MAJOR 0)
set (LIBTE_VERSION_MINOR 8)
set (LIBTE_VERSION_STRING "${LIBTE_VERSION_MAJOR}.${LIBTE_VERSION_MINOR}")

# cmake adds -rdynamic when linking dynamic libraries on Linux, work around
IF (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
	SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
ENDIF ()


OPTION (OPTION_SHARED
	"Build as shared libraries"
	OFF
)
OPTION (OPTION_DOC
	"Generate documentation"
	OFF
)

# Macro for joining strings with space separator
MACRO(CONCAT varname)
	FOREACH(ii ${ARGN})
		SET(${varname} "${${varname}} ${ii}" )
	ENDFOREACH()
ENDMACRO(CONCAT)

IF (CMAKE_COMPILER_IS_GNUCC)
	CONCAT (CMAKE_C_FLAGS -std=gnu99 -Wall)
	CONCAT (CMAKE_CXX_FLAGS -std=c++98)
	IF (NOT APPLE)
		CONCAT (CMAKE_C_FLAGS -fexec-charset=utf-8 -finput-charset=utf-8)
		CONCAT (CMAKE_CXX_FLAGS -fexec-charset=utf-8 -finput-charset=utf-8)
	ENDIF (NOT APPLE)
ENDIF (CMAKE_COMPILER_IS_GNUCC)

IF (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	ADD_DEFINITIONS(-DNDEBUG)
	IF (CMAKE_COMPILER_IS_GNUCC AND NOT APPLE)
		CONCAT (CMAKE_C_FLAGS "-fvisibility=hidden")
		CONCAT (CMAKE_CXX_FLAGS "-fvisibility=hidden")
		CONCAT (CMAKE_C_CREATE_SHARED_LIBRARY "-fvisibility=hidden")
		CONCAT (CMAKE_EXE_LINKER_FLAGS "-s")
		CONCAT (CMAKE_EXE_LINKER_FLAGS "-Xlinker --no-export-dynamic")
		CONCAT (CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "-Xlinker --strip-all")

		CONCAT (CMAKE_C_FLAGS_MINSIZEREL -flto -flto-partition=none -fomit-frame-pointer -ffast-math -mfpmath=sse)
		CONCAT (CMAKE_CXX_FLAGS_MINSIZEREL -flto -flto-partition=none -fomit-frame-pointer -ffast-math -mfpmath=sse)
		CONCAT (CMAKE_EXE_LINKER_FLAGS_MINSIZEREL -Xlinker --strip-all)

	ENDIF (CMAKE_COMPILER_IS_GNUCC AND NOT APPLE)
ENDIF ()


IF (OPTION_DOC)
	ADD_SUBDIRECTORY(doc)
ENDIF ()
ADD_SUBDIRECTORY(libte)
ADD_SUBDIRECTORY(flte)
ADD_SUBDIRECTORY(flterm)
