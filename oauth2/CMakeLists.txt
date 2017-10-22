# OAuth 2 Authentication Method plugin

SET(PLUGIN_NAME OAuth2)

########################################################
# Packages
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

IF(QGIS_APP_NAME)
  #message(STATUS "Building OAuth2 plugin in QGIS src tree")
  # we are in QGIS src tree
  SET(IN_QGIS_SRC TRUE)
  #use its version
  SET(QGIS_VERSION "${COMPLETE_VERSION}")
ENDIF()

IF(NOT QGIS_VERSION)
  MESSAGE(FATAL_ERROR "QGIS_VERSION undetermined")
ENDIF()

IF("${QGIS_VERSION}" VERSION_LESS "2.99.0")
  SET(QGIS2 TRUE)
  MESSAGE(STATUS "QGIS target version for ${PLUGIN_NAME} plugin set to 2.x")
ELSE()
  SET(QGIS2 FALSE)
  MESSAGE(STATUS "QGIS target version for ${PLUGIN_NAME} plugin set to 3.x")
ENDIF()

IF(QGIS2)
  SET(QT_USE_QTSCRIPT 1)
  INCLUDE(${QT_USE_FILE})
  IF(NOT QJSON_FOUND)
    FIND_PACKAGE(QJSON REQUIRED)
  ENDIF()
ENDIF()

IF(NOT QCA_FOUND)
  FIND_PACKAGE(QCA REQUIRED)
ENDIF()

IF(NOT QTKEYCHAIN_FOUND)
  FIND_PACKAGE(QtKeychain REQUIRED)
ENDIF()

OPTION(WITH_INTERNAL_O2 "Download and locally include source of o2 library" ON)

IF(WITH_INTERNAL_O2)
  INCLUDE(DownloadO2)
  SET(O2_SOURCE_DIR ${O2_INCLUDE_DIR})
ELSE()
  FIND_PACKAGE(O2 REQUIRED)
ENDIF()

########################################################
# Source files

SET(PLUGIN_SRCS
  qgso2.cpp
  qgsauthoauth2config.cpp
  qgsauthoauth2method.cpp
  qgsauthoauth2edit.cpp
  qjsonwrapper/Json.cpp
)
IF(WITH_INTERNAL_O2)
  SET(O2_SRCS
    ${O2_SOURCE_DIR}/o0baseauth.cpp
    ${O2_SOURCE_DIR}/o0keychainstore.cpp
    ${O2_SOURCE_DIR}/o0settingsstore.cpp
    ${O2_SOURCE_DIR}/o2.cpp
    ${O2_SOURCE_DIR}/o2reply.cpp
    ${O2_SOURCE_DIR}/o2replyserver.cpp
    ${O2_SOURCE_DIR}/o2requestor.cpp
    ${O2_SOURCE_DIR}/o2simplecrypt.cpp
  )
  SET(PLUGIN_SRCS ${PLUGIN_SRCS} ${O2_SRCS})
ENDIF()

SET(PLUGIN_HDRS
  qgso2.h
  qgsauthoauth2config.h
  qgsauthoauth2method.h
  qgsauthoauth2edit.h
  qjsonwrapper/Json.h
)
IF(WITH_INTERNAL_O2)
  SET(O2_HDRS
    ${O2_INCLUDE_DIR}/o0abstractstore.h
    ${O2_INCLUDE_DIR}/o0baseauth.h
    ${O2_INCLUDE_DIR}/o0export.h
    ${O2_INCLUDE_DIR}/o0globals.h
    ${O2_INCLUDE_DIR}/o0keychainstore.h
    ${O2_INCLUDE_DIR}/o0requestparameter.h
    ${O2_INCLUDE_DIR}/o0settingsstore.h
    ${O2_INCLUDE_DIR}/o0simplecrypt.h
    ${O2_INCLUDE_DIR}/o2.h
    ${O2_INCLUDE_DIR}/o2reply.h
    ${O2_INCLUDE_DIR}/o2replyserver.h
    ${O2_INCLUDE_DIR}/o2requestor.h
  )
  SET(PLUGIN_HDRS ${PLUGIN_HDRS} ${O2_HDRS})
ENDIF()

SET(PLUGIN_MOC_HDRS
  qgso2.h
  qgsauthoauth2config.h
  qgsauthoauth2method.h
  qgsauthoauth2edit.h
)
IF(WITH_INTERNAL_O2)
  SET(O2_MOC_HDRS
    ${O2_INCLUDE_DIR}/o0abstractstore.h
    ${O2_INCLUDE_DIR}/o0baseauth.h
    ${O2_INCLUDE_DIR}/o0keychainstore.h
    ${O2_INCLUDE_DIR}/o0settingsstore.h
    ${O2_INCLUDE_DIR}/o2.h
    ${O2_INCLUDE_DIR}/o2reply.h
    ${O2_INCLUDE_DIR}/o2replyserver.h
    ${O2_INCLUDE_DIR}/o2requestor.h
  )
  SET(PLUGIN_MOC_HDRS ${PLUGIN_MOC_HDRS} ${O2_MOC_HDRS})
ENDIF()

IF(QGIS2)
  # QgsPasswordLineEdit is in QGIS 2.99 at 85776a1
  SET(PLUGIN_SRCS ${PLUGIN_SRCS} qgis2-ui/qgspasswordlineedit.cpp)
  SET(PLUGIN_HDRS ${PLUGIN_HDRS} qgis2-ui/qgspasswordlineedit.h)
  SET(PLUGIN_MOC_HDRS ${PLUGIN_MOC_HDRS} qgis2-ui/qgspasswordlineedit.h)
ENDIF()

SET(PLUGIN_UIS qgsauthoauth2edit.ui)

SET(PLUGIN_RCCS oauth2_resources.qrc)

# Common includes
INCLUDE_DIRECTORIES (
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)
INCLUDE_DIRECTORIES (SYSTEM
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${O2_INCLUDE_DIR}
)
IF(QGIS2)
  INCLUDE_DIRECTORIES (SYSTEM
    ${QJSON_INCLUDE_DIR}
  )
  INCLUDE_DIRECTORIES (
    qgis2-ui
  )
ENDIF()

IF(IN_QGIS_SRC)
  ### start: when in QGIS/src/auth ###
  INCLUDE_DIRECTORIES (
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/core/auth
    ${CMAKE_SOURCE_DIR}/src/core/geometry
    ${CMAKE_SOURCE_DIR}/src/auth/oauth2
    ${CMAKE_SOURCE_DIR}/src/gui
    ${CMAKE_SOURCE_DIR}/src/gui/auth
    ${CMAKE_BINARY_DIR}/src/core
    ${CMAKE_BINARY_DIR}/src/gui
    ${CMAKE_BINARY_DIR}/src/ui
    ${CMAKE_BINARY_DIR}/src/ui/auth
    ${CMAKE_BINARY_DIR}/src/auth/oauth2
  )
  ### end: when in QGIS/src/auth ###
ELSE(IN_QGIS_SRC)
  ### start: when standalone ###
  INCLUDE_DIRECTORIES (SYSTEM
    ${Qt5Sql_INCLUDE_DIRS}
    ${QT_INCLUDE_DIR}
    ${QGIS_INCLUDE_DIR}
  )
  ### end: when standalone ###
ENDIF(IN_QGIS_SRC)


############################################################
# Generate files

IF(WITH_INTERNAL_O2 AND CMAKE_GENERATOR MATCHES "Ninja")
  # handle `cmake --help-policy CMP0058`
  #  "Ninja requires custom command byproducts to be explicit,"
  #   which the downloaded o2 source files are not. Add phony command target...
  ADD_CUSTOM_COMMAND(
    OUTPUT ${O2_HDRS}
    COMMAND
  )
ENDIF()

QT4_WRAP_UI(PLUGIN_UIS_H ${PLUGIN_UIS})

QT4_WRAP_CPP(PLUGIN_MOC_SRCS ${PLUGIN_MOC_HDRS})

QT4_ADD_RESOURCES(PLUGIN_RCC_SRCS ${PLUGIN_RCCS})

############################################################
# Set up plugin targets

ADD_LIBRARY (oauth2authmethod MODULE
  ${PLUGIN_SRCS}
  ${PLUGIN_HDRS}
  ${PLUGIN_MOC_SRCS}
  ${PLUGIN_RCC_SRCS}
  ${PLUGIN_UIS_H}
)

# For unit testing
IF(ENABLE_TESTS)
  ADD_LIBRARY (oauth2authmethod_static STATIC
    ${PLUGIN_SRCS}
    ${PLUGIN_HDRS}
    ${PLUGIN_MOC_SRCS}
    ${PLUGIN_RCC_SRCS}
    ${PLUGIN_UIS_H}
  )
ENDIF(ENABLE_TESTS)

# Common linked libs
SET(PLUGIN_TARGET_LIBS ${QTKEYCHAIN_LIBRARY})

IF(NOT WITH_INTERNAL_O2)
  IF(NOT "${O2_LIBRARY}" STREQUAL "")
    # prefer dynamic linking
    SET(PLUGIN_TARGET_LIBS ${PLUGIN_TARGET_LIBS} ${O2_LIBRARY})
  ELSE()
    SET(PLUGIN_TARGET_LIBS ${PLUGIN_TARGET_LIBS} ${O2_LIBRARY_STATIC})
  ENDIF()
  #message(STATUS "PLUGIN_TARGET_LIBS: ${PLUGIN_TARGET_LIBS}")
ENDIF()

IF(QGIS2)
  SET(PLUGIN_TARGET_LIBS
    ${QT_QTSCRIPT_LIBRARY}
    ${QJSON_LIBRARIES}
    ${PLUGIN_TARGET_LIBS}
  )
ENDIF()

IF(IN_QGIS_SRC)
  # in QGIS source tree
  SET(PLUGIN_TARGET_LIBS
    qgis_core
    qgis_gui
    ${PLUGIN_TARGET_LIBS}
  )
ELSE(IN_QGIS_SRC)
  # outside QGIS source tree
  SET(PLUGIN_TARGET_LIBS
    ${QGIS_CORE_LIBRARY}
    ${QGIS_GUI_LIBRARY}
    ${QT_QTMAIN_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTSVG_LIBRARY}
    ${PLUGIN_TARGET_LIBS}
  )
ENDIF(IN_QGIS_SRC)

TARGET_LINK_LIBRARIES (oauth2authmethod
  ${PLUGIN_TARGET_LIBS}
)

IF(ENABLE_TESTS)
  TARGET_LINK_LIBRARIES (oauth2authmethod_static
    ${PLUGIN_TARGET_LIBS}
  )
ENDIF(ENABLE_TESTS)

if(WIN32)
  add_definitions(-DO2_DLL_EXPORT)
endif()


########################################################
# Install

# don't install static variant, as it's only for unit testing from build directory
INSTALL(TARGETS oauth2authmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
