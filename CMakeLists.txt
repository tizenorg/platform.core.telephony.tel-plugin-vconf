CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(vconf-plugin C)

### Global setting ###
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${prefix}/lib")
SET(INCLUDEDIR "\${prefix}/include")
SET(PKGCONFIGDIR "${PREFIX}/${LIB_INSTALL_DIR}/pkgconfig" CACHE PATH PKGCONFIGDIR)
SET(CMAKE_INSTALL_PREFIX "${PREFIX}")

# Set required packages
INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED vconf glib-2.0 gthread-2.0 tcore dlog)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement -Wmissing-declarations -Wredundant-decls -Wcast-align")

ADD_DEFINITIONS("-DFEATURE_DLOG_DEBUG")
ADD_DEFINITIONS("-DTCORE_LOG_TAG=\"TEL_PLUGIN_VCONF\"")
ADD_DEFINITIONS("-DPLUGIN_VERSION=${VERSION}")

MESSAGE(${CMAKE_C_FLAGS})
MESSAGE(${CMAKE_EXE_LINKER_FLAGS})

SET(SRCS
		src/desc-vconf.c
)


# library build
ADD_LIBRARY(vconf-plugin SHARED ${SRCS})
TARGET_LINK_LIBRARIES(vconf-plugin ${pkgs_LDFLAGS})
SET_TARGET_PROPERTIES(vconf-plugin PROPERTIES PREFIX "" OUTPUT_NAME vconf-plugin)



# install
INSTALL(TARGETS vconf-plugin
	LIBRARY DESTINATION ${LIB_INSTALL_DIR}/telephony/plugins)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION /usr/share/license RENAME tel-plugin-vconf)
