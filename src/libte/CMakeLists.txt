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

FOREACH(ii libte.h symbol.h macros.h LibTE.hpp)
	LIST(APPEND apiheaders ${CMAKE_CURRENT_SOURCE_DIR}/${ii} )
ENDFOREACH()

INSTALL (TARGETS te
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)
INSTALL (FILES ${apiheaders}
	DESTINATION include/libte)

#
# Generate libte.pc file for use with pkg-config
#
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libte.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libte.pc @ONLY)
INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/libte.pc
	DESTINATION lib/pkgconfig)
