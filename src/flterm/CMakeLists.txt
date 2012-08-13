SET(flte_sources
	Flx_PseudoTerm.cxx
	Flx_ScrolledTerm.cxx
	Flxi_BasicTerm.cxx
	fontrender.c
	strutil.c
	pty/pty.c
	pty/term.c
)

CONCAT (CMAKE_EXE_LINKER_FLAGS "-Wl,-x")
#CONCAT (CMAKE_EXE_LINKER_FLAGS "-Xlinker --no-export-dynamic")

CONCAT (CMAKE_C_CREATE_SHARED_LIBRARY "-Wl,-x")
#CONCAT (CMAKE_C_CREATE_SHARED_LIBRARY "-Xlinker --no-export-dynamic")

INCLUDE_DIRECTORIES( ".." )

# Add freetype
FIND_PACKAGE( Freetype REQUIRED )
INCLUDE_DIRECTORIES( ${FREETYPE_INCLUDE_DIRS} )

# Add FLTK (currently broken on wheezy?)
IF (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	SET(FLTK_LIBRARIES "fltk")
ELSE ()
	SET (FLTK_SKIP_FORMS TRUE)
	SET (FLTK_SKIP_OPENGL TRUE)
	SET (FLTK_SKIP_IMAGES TRUE)
	SET (FLTK_SKIP_FLUID TRUE)
	FIND_PACKAGE( FLTK REQUIRED )
	INCLUDE_DIRECTORIES( ${FLTK_INCLUDE_DIRS} )
#	MESSAGE( "FLTK_LIBRARIES: ${FLTK_LIBRARIES}")
	SET (FLTK_LIBRARIES "/opt/local/lib/libfltk.dylib")
ENDIF()

ADD_LIBRARY(flte STATIC ${flte_sources})
TARGET_LINK_LIBRARIES(flte te ${FREETYPE_LIBRARIES})

ADD_EXECUTABLE(flterm "term_sh.cxx")
TARGET_LINK_LIBRARIES( flterm flte ${FLTK_LIBRARIES})
ADD_DEFINITIONS(-D_GNU_SOURCE)

#FOREACH(ii libte.h symbols.h LibTE.hpp)
#	LIST(APPEND apiheaders ${CMAKE_CURRENT_SOURCE_DIR}/${ii} )
#ENDFOREACH()
#
INSTALL (TARGETS flte
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

INSTALL (TARGETS flterm
	RUNTIME DESTINATION bin)

#INSTALL (FILES ${apiheaders}
#	DESTINATION include)

#
# Generate libflte.pc file for use with pkg-config
#
#CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libflte.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libflte.pc @ONLY)
#INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/libflte.pc
#	DESTINATION lib/pkgconfig)
