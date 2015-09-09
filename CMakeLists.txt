CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(account C)

INCLUDE(FindPkgConfig)
pkg_check_modules(account_pkgs REQUIRED
	dlog
	capi-base-common
	libxml-2.0
	pkgmgr-info
	glib-2.0
	accounts-svc
	capi-appfw-app-manager
	libtzplatform-config
)

FOREACH(flag ${account_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET (SRCS
	src/account.c
)

SET(EXTRA_CFLAGS  "${EXTRA_CFLAGS} -Wall" )
SET(extapi "-fvisibility=hidden")

## SET C COMPILER FLAGS
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} ${extapi}")

## SET CPP COMPILER FLAGS
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
#SET(CMAKE_CXX_FLAGS "${OSP_DEBUG_FLAGS} ${OSP_OPT_FLAGS} ${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} ${OSP_COMPILER_FLAGS}")

## SET LINKER FLAGS
SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed")

## Create Library
ADD_LIBRARY (${PROJECT_NAME} SHARED ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${account_pkgs_LDFLAGS} )

#CONFIGURE_FILE(account-parser.pc.in ${CMAKE_SOURCE_DIR}/account-parser.pc @ONLY)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/account-parser.pc DESTINATION lib/pkgconfig)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${SYSCONFDIR}/package-manager/parserlib)
