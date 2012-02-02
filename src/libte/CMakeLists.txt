INCLUDE (CheckIncludeFiles)
INCLUDE (CheckCSourceCompiles)
INCLUDE (CheckFunctionExists)

#### Check for basic functionality ####
CHECK_INCLUDE_FILES (malloc.h HAVE_MALLOC_H)
CHECK_INCLUDE_FILES (stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES (sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILES (uchar.h HAVE_UCHAR_H)
CHECK_C_SOURCE_COMPILES ("
static __thread int a;
int main(int argc, char * argv[]){return 0;}
" FC_HAVE_NATIVE_TLS)
CHECK_FUNCTION_EXISTS(mmap HAVE_MMAP)
CHECK_FUNCTION_EXISTS(calloc HAVE_CALLOC)
CHECK_FUNCTION_EXISTS(realloc HAVE_REALLOC)

IF (${CMAKE_COMPILER_IS_GNUCC})
	SET (FC_HAVE_GCC 1)
ENDIF ()

MACRO(CHECK_PREPROC ppexpr varname)
	CHECK_C_SOURCE_COMPILES ("
	#if (${ppexpr})
		int main(int argc, char * argv[]){return 0;}
	#else
	#	error Preprocessing test false
	#endif
	" ${varname})
ENDMACRO(CHECK_PREPROC)

#### Check target SYSTEM type ####
IF (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	SET (HAVE_LINUX 1)
ENDIF ()


#### Write fc_config.h ####
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.template ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

SET(libte_sources
	actions.c
	buffer.c
	bufferrow.c
	charsets.c
	history.c
	libte.c
	misc.c
	parser.c
	parser_states.c
	viewport.c
	viewport_dirty.c
)

CONCAT (CMAKE_EXE_LINKER_FLAGS "-Wl,-x")
#CONCAT (CMAKE_EXE_LINKER_FLAGS "-Xlinker --no-export-dynamic")

CONCAT (CMAKE_C_CREATE_SHARED_LIBRARY "-Wl,-x")
#CONCAT (CMAKE_C_CREATE_SHARED_LIBRARY "-Xlinker --no-export-dynamic")

IF (OPTION_SHARED)
	ADD_LIBRARY(te SHARED ${libte_sources})
	SET_TARGET_PROPERTIES(te
		PROPERTIES
		SOVERSION	${LIBTE_VERSION_MAJOR}
		VERSION		${LIBTE_VERSION_MAJOR}.${LIBTE_VERSION_MINOR})
ELSE (OPTION_SHARED)
	ADD_LIBRARY(te STATIC ${libte_sources})
ENDIF (OPTION_SHARED)

FOREACH(ii libte.h symbols.h config.h LibTE.hpp)
	LIST(APPEND apiheaders ${CMAKE_CURRENT_SOURCE_DIR}/${ii} )
ENDFOREACH()

INSTALL (TARGETS te
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)
INSTALL (FILES ${apiheaders}
	DESTINATION include)

#
# Generate libte.pc file for use with pkg-config
#
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libte.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libte.pc @ONLY)
INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/libte.pc
	DESTINATION lib/pkgconfig)
