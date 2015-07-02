# for package file
SET(dependents "capi-base-common glib-2.0 gio-2.0 dlog sqlite3 db-util security-server libsystemd-daemon libtzplatform-config")
SET(fw_svc "media-controller-svc")

PROJECT(${fw_svc})

SET(CMAKE_INSTALL_PREFIX ${prefix})
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(VERSION ${version})

SET(INC_DIR include)
INCLUDE_DIRECTORIES(${INC_DIR})

INCLUDE(FindPkgConfig)
pkg_check_modules(${fw_svc} REQUIRED ${dependents})
FOREACH(flag ${${fw_svc}_CFLAGS})
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

aux_source_directory(. SOURCES_SVC)
ADD_LIBRARY(${fw_svc} SHARED ${SOURCES_SVC})

TARGET_LINK_LIBRARIES(${fw_svc} ${${fw_svc}_LDFLAGS})

SET_TARGET_PROPERTIES(${fw_svc}
     PROPERTIES
     VERSION ${FULLVER}
     SOVERSION ${MAJORVER}
     CLEAN_DIRECT_OUTPUT 1
)

INSTALL(TARGETS ${fw_svc} DESTINATION lib)
ADD_SUBDIRECTORY(daemon)
