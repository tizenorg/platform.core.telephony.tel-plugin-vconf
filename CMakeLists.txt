CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(vconf-plugin C)

### Global setting ###
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR ${LIB_INSTALL_DIR})
SET(INCLUDEDIR "\${prefix}/include")
SET(PKGCONFIGDIR "${PREFIX}/lib/pkgconfig" CACHE PATH PKGCONFIGDIR)
SET(CMAKE_INSTALL_PREFIX "${PREFIX}")

# Set required packages
INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED vconf glib-2.0 tcore)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/)

SET(ADDITIONAL_CFLAGS "-Wall -Wno-array-bounds -Wno-empty-body -Wno-ignored-qualifiers -Wshadow -Wwrite-strings -Wswitch-default -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-strict-aliasing -fno-unroll-loops -fsigned-char -fstrict-overflow -Wno-unused-but-set-parameter -Wno-unused-but-set-variable -Wcast-qual")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wextra -fvisibility=hidden -fPIC")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement -Wmissing-declarations -Wredundant-decls -Wcast-align")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADDITIONAL_CFLAGS}")

### Purge unused code ###
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdata-sections -ffunction-sections -Wl,--gc-sections")

ADD_DEFINITIONS("-DFEATURE_TLOG_DEBUG")
ADD_DEFINITIONS("-DTCORE_LOG_TAG=\"SP_VCONF\"")
ADD_DEFINITIONS("-DPLUGIN_VERSION=${VERSION}")
ADD_DEFINITIONS("-DEXPORT_API=__attribute__((visibility(\"default\")))")

IF (TIZEN_FEATURE_DSDS)
        ADD_DEFINITIONS("-DTIZEN_FEATURE_DSDS")
ENDIF (TIZEN_FEATURE_DSDS)
IF (TIZEN_FEATURE_CDMA)
	ADD_DEFINITIONS("-DTIZEN_FEATURE_CDMA")
ENDIF (TIZEN_FEATURE_CDMA)

MESSAGE(${CMAKE_C_FLAGS})
MESSAGE(${pkgs_LDFLAGS})

SET(SRCS
		src/desc-vconf.c
)


# library build
ADD_LIBRARY(vconf-plugin SHARED ${SRCS})
TARGET_LINK_LIBRARIES(vconf-plugin ${pkgs_LDFLAGS})
SET_TARGET_PROPERTIES(vconf-plugin PROPERTIES PREFIX "" OUTPUT_NAME vconf-plugin)



# install
INSTALL(TARGETS vconf-plugin
		LIBRARY DESTINATION ${LIBDIR}/telephony/plugins)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION /usr/share/license RENAME tel-plugin-vconf)
