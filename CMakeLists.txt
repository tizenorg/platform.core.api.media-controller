CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(Services
        "application"
        "base"
        "content"
        "location"
        "media"
        "messaging"
        "network"
        "social"
        "telephony"
        "system"
   )


# project
SET(project_prefix "capi")
SET(prefix "/usr")
SET(version "0.0.1")
SET(maintainer "Haejeong Kim <backto.kim@samsung.com>")
SET(description "A Media Controller library in Tizen Native API")
SET(service "media")
SET(submodule "controller")

# for package file
SET(dependents "capi-base-common glib-2.0 gio-2.0 dlog sqlite3 db-util aul bundle")
SET(fw_name "${project_prefix}-${service}-${submodule}")

PROJECT(${fw_name})

SET(CMAKE_INSTALL_PREFIX ${prefix})
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(VERSION ${version})

SET(INC_DIR include)
SET(SVC_DIR svc)

INCLUDE_DIRECTORIES(${INC_DIR} ${SVC_DIR})

INCLUDE(FindPkgConfig)
pkg_check_modules(${fw_name} REQUIRED ${dependents})
FOREACH(flag ${${fw_name}_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIC -Wall -Werror")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")

IF("${ARCH}" MATCHES "^arm.*")
    ADD_DEFINITIONS("-DTARGET")
ENDIF("${ARCH}" MATCHES "^arm.*")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DTIZEN_DEBUG")
ADD_DEFINITIONS("-D_FILE_OFFSET_BITS=64")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -Wl,--rpath=/usr/lib")

aux_source_directory(src SOURCES)
ADD_LIBRARY(${fw_name} SHARED ${SOURCES})

TARGET_LINK_LIBRARIES(${fw_name} media-controller-svc ${${fw_name}_LDFLAGS})

SET_TARGET_PROPERTIES(${fw_name}
     PROPERTIES
     VERSION ${FULLVER}
     SOVERSION ${MAJORVER}
     CLEAN_DIRECT_OUTPUT 1
)

INSTALL(TARGETS ${fw_name} DESTINATION lib)
INSTALL(
        DIRECTORY ${INC_DIR}/ DESTINATION include/${service}
        FILES_MATCHING
        PATTERN "*_private.h" EXCLUDE
        PATTERN "*_db.h" EXCLUDE
        PATTERN "${INC_DIR}/*.h"
        )

SET(PC_NAME ${fw_name})
SET(PC_REQUIRED ${dependents})
SET(PC_LDFLAGS -l${fw_name})
SET(PC_CFLAGS -I\${includedir}/${service})

CONFIGURE_FILE(
    ${fw_name}.pc.in
    ${CMAKE_CURRENT_SOURCE_DIR}/${fw_name}.pc
    @ONLY
)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${fw_name}.pc DESTINATION lib/pkgconfig)

ADD_SUBDIRECTORY(svc)
ADD_SUBDIRECTORY(test/client_test)
ADD_SUBDIRECTORY(test/server_test)

IF(UNIX)

ADD_CUSTOM_TARGET (distclean @echo cleaning for source distribution)
ADD_CUSTOM_COMMAND(
        DEPENDS clean
        COMMENT "distribution clean"
        COMMAND find
        ARGS    .
        -not -name config.cmake -and \(
        -name tester.c -or
        -name Testing -or
        -name CMakeFiles -or
        -name cmake.depends -or
        -name cmake.check_depends -or
        -name CMakeCache.txt -or
        -name cmake.check_cache -or
        -name *.cmake -or
        -name Makefile -or
        -name core -or
        -name core.* -or
        -name gmon.out -or
        -name install_manifest.txt -or
        -name *.pc -or
        -name *~ \)
        | grep -v TC | xargs rm -rf
        TARGET  distclean
        VERBATIM
)

ENDIF(UNIX)
