CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (LIBTE)
# The version number.
set (LIBTE_VERSION_MAJOR 0)
set (LIBTE_VERSION_MINOR 9)
set (LIBTE_VERSION_STRING "${LIBTE_VERSION_MAJOR}.${LIBTE_VERSION_MINOR}")


OPTION (OPTION_SHARED
	"Build as shared libraries"
	OFF
)
OPTION (OPTION_DOC
	"Generate documentation"
	OFF
)
ADD_DEFINITIONS(-Wall)

# Macro for joining strings with space separator
MACRO(CONCAT varname)
	FOREACH(ii ${ARGN})
		SET(${varname} "${${varname}} ${ii}" )
	ENDFOREACH()
ENDMACRO(CONCAT)

CONCAT (CMAKE_C_FLAGS -std=gnu99 -fexec-charset=utf-8 -finput-charset=utf-8)
CONCAT (CMAKE_CXX_FLAGS -std=gnu++98 -fexec-charset=utf-8 -finput-charset=utf-8)

IF (NOT DEBUG)
	ADD_DEFINITIONS(-DNDEBUG)
	IF (CMAKE_COMPILER_IS_GNUCC AND NOT APPLE)
		CONCAT (CMAKE_C_FLAGS "-fvisibility=hidden")
		CONCAT (CMAKE_CXX_FLAGS "-fvisibility=hidden")
		CONCAT (CMAKE_C_CREATE_SHARED_LIBRARY "-fvisibility=hidden")
		CONCAT (CMAKE_EXE_LINKER_FLAGS "-s")
		CONCAT (CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "-Xlinker --strip-all")
	ENDIF (CMAKE_COMPILER_IS_GNUCC AND NOT APPLE)
ENDIF (NOT DEBUG)

IF (OPTION_DOC)
	ADD_SUBDIRECTORY(doc)
ENDIF ()
ADD_SUBDIRECTORY(src/libte)
ADD_SUBDIRECTORY(src/flterm)
