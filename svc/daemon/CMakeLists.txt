SET(fw_daemon "mediacontroller")

INCLUDE(FindPkgConfig)
pkg_check_modules(${fw_daemon} REQUIRED glib-2.0)

FOREACH(flag ${${fw_daemon}_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIE -Wall")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -Wl,--hash-style=both -pie")

aux_source_directory(. sources)
FOREACH(src ${sources})
    GET_FILENAME_COMPONENT(src_name ${src} NAME_WE)
    MESSAGE("${src_name}")
    ADD_EXECUTABLE(${fw_daemon} ${src})
    TARGET_LINK_LIBRARIES(${fw_daemon} media-controller-svc ${${fw_daemon}_LDFLAGS})
ENDFOREACH()

INSTALL(TARGETS ${fw_daemon} DESTINATION bin)
