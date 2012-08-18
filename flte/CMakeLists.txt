OPTION (FLTE_ENABLE_FREETYPE
	"enable freetype font rendering"
	OFF
)

SET(flte_sources
	Flx_SlaveIO.cxx
	Flx_ScrolledTerm.cxx
	Flxi_BasicTerm.cxx
	strutil.c
)

CONCAT (CMAKE_EXE_LINKER_FLAGS "-Wl,-x")
#CONCAT (CMAKE_EXE_LINKER_FLAGS "-Xlinker --no-export-dynamic")

CONCAT (CMAKE_C_CREATE_SHARED_LIBRARY "-Wl,-x")
#CONCAT (CMAKE_C_CREATE_SHARED_LIBRARY "-Xlinker --no-export-dynamic")

INCLUDE_DIRECTORIES( ".." )
INCLUDE_DIRECTORIES( "${LIBTE_BINARY_DIR}/libte" )

IF (FLTE_ENABLE_FREETYPE)
	LIST(APPEND flte_sources "fontrender.c")

	# Add freetype
	FIND_PACKAGE( Freetype REQUIRED )
	INCLUDE_DIRECTORIES( ${FREETYPE_INCLUDE_DIRS} )
ENDIF (FLTE_ENABLE_FREETYPE)

# Add FLTK (currently broken on wheezy?)
IF (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	SET(FLTK_LIBRARIES "fltk")
ELSE ()
#	SET (FLTK_SKIP_FORMS TRUE)
#	SET (FLTK_SKIP_OPENGL TRUE)
#	SET (FLTK_SKIP_IMAGES TRUE)
#	SET (FLTK_SKIP_FLUID TRUE)
#	FIND_PACKAGE( FLTK REQUIRED )
#	INCLUDE_DIRECTORIES( ${FLTK_INCLUDE_DIRS} )
#	MESSAGE( "FLTK_LIBRARIES: ${FLTK_LIBRARIES}")
#	SET (FLTK_LIBRARIES "/opt/local/lib/libfltk.dylib")

	SET (FLTK_INCLUDE_DIRS "/opt/local/include")
	SET (FLTK_LIBRARIES "/opt/local/lib/libfltk.dylib")
ENDIF()

INCLUDE_DIRECTORIES(${FLTK_INCLUDE_DIRS})
ADD_LIBRARY(flte STATIC ${flte_sources})
TARGET_LINK_LIBRARIES(flte te ${FREETYPE_LIBRARIES})

FOREACH(ii Flxi_BasicTerm.hpp Flx_ScrolledTerm.hpp Flx_IResizableParent.hpp Flx_SlaveIO.hpp)
	LIST(APPEND apiheaders ${CMAKE_CURRENT_SOURCE_DIR}/${ii} )
ENDFOREACH()

INSTALL (TARGETS flte
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

INSTALL (FILES ${apiheaders}
	DESTINATION include/flte)

#
# Generate libflte.pc file for use with pkg-config
#
#CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libflte.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libflte.pc @ONLY)
#INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/libflte.pc
#	DESTINATION lib/pkgconfig)
