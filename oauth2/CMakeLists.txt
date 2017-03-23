# OAuth 2 Authentication Method plugin

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
  MESSAGE(STATUS "QGIS target version for OAuth2 plugin set to 2.x")
ELSE()
  SET(QGIS2 FALSE)
  MESSAGE(STATUS "QGIS target version for OAuth2 plugin set to 3.x")
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

OPTION(WITH_INTERNAL_O2 "Locally build o2 library as external project" ON)

IF(WITH_INTERNAL_O2)
  INCLUDE(External_O2)
ELSE()
  FIND_PACKAGE(O2 REQUIRED)
ENDIF()

########################################################
# Files

SET(OAUTH2_SRCS
  qgso2.cpp
  qgsauthoauth2config.cpp
  qgsauthoauth2method.cpp
  qgsauthoauth2edit.cpp
  qjsonwrapper/Json.cpp
)

SET(OAUTH2_HDRS
  qgso2.h
  qgsauthoauth2config.h
  qgsauthoauth2method.h
  qgsauthoauth2edit.h
  qjsonwrapper/Json.h
)

SET(OAUTH2_MOC_HDRS
  qgso2.h
  qgsauthoauth2config.h
  qgsauthoauth2method.h
  qgsauthoauth2edit.h
)

IF(QGIS2)
  # QgsPasswordLineEdit is in QGIS 2.99 at 85776a1
  SET(OAUTH2_SRCS ${OAUTH2_SRCS} qgis2-ui/qgspasswordlineedit.cpp)
  SET(OAUTH2_HDRS ${OAUTH2_HDRS} qgis2-ui/qgspasswordlineedit.h)
  SET(OAUTH2_MOC_HDRS ${OAUTH2_MOC_HDRS} qgis2-ui/qgspasswordlineedit.h)
ENDIF()

SET(OAUTH2_UIS qgsauthoauth2edit.ui)

SET(OAUTH2_RCCS oauth2_resources.qrc)

# Common includes
INCLUDE_DIRECTORIES (
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)
INCLUDE_DIRECTORIES (SYSTEM
  ${QCA_INCLUDE_DIR}
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


QT4_WRAP_UI(OAUTH2_UIS_H ${OAUTH2_UIS})

QT4_WRAP_CPP(OAUTH2_MOC_SRCS ${OAUTH2_MOC_HDRS})

QT4_ADD_RESOURCES(OAUTH2_RCC_SRCS ${OAUTH2_RCCS})

ADD_LIBRARY (oauth2authmethod MODULE
  ${OAUTH2_SRCS}
  ${OAUTH2_HDRS}
  ${OAUTH2_MOC_SRCS}
  ${OAUTH2_RCC_SRCS}
  ${OAUTH2_UIS_H}
)

# For unit testing
ADD_LIBRARY (oauth2authmethod_static STATIC
  ${OAUTH2_SRCS}
  ${OAUTH2_HDRS}
  ${OAUTH2_MOC_SRCS}
  ${OAUTH2_RCC_SRCS}
  ${OAUTH2_UIS_H}
)

# Common linked libs
IF(NOT "${O2_LIBRARY_STATIC}" STREQUAL "")
  # prefer static linking, so newer o2 lib does not need separately packaged
  SET(OAUTH2_TARGET_LIBS ${O2_LIBRARY_STATIC})
ELSE()
  SET(OAUTH2_TARGET_LIBS ${O2_LIBRARY})
ENDIF()

#message(STATUS "OAUTH2_TARGET_LIBS: ${OAUTH2_TARGET_LIBS}")

IF(QGIS2)
  SET(OAUTH2_TARGET_LIBS
    ${QT_QTSCRIPT_LIBRARY}
    ${QJSON_LIBRARIES}
    ${OAUTH2_TARGET_LIBS}
  )
ENDIF()

IF(IN_QGIS_SRC)
  # in QGIS source tree
  SET(OAUTH2_TARGET_LIBS
    qgis_core
    qgis_gui
    ${OAUTH2_TARGET_LIBS}
  )
ELSE(IN_QGIS_SRC)
  # outside QGIS source tree
  SET(OAUTH2_TARGET_LIBS
    ${QGIS_CORE_LIBRARY}
    ${QGIS_GUI_LIBRARY}
    ${QT_QTMAIN_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTSVG_LIBRARY}
    ${OAUTH2_TARGET_LIBS}
  )
ENDIF(IN_QGIS_SRC)

TARGET_LINK_LIBRARIES (oauth2authmethod
  ${OAUTH2_TARGET_LIBS}
)
TARGET_LINK_LIBRARIES (oauth2authmethod_static
  ${OAUTH2_TARGET_LIBS}
)

IF(NOT WITH_SYSTEM_O2)
  ADD_DEPENDENCIES(oauth2authmethod external_o2)
  ADD_DEPENDENCIES(oauth2authmethod_static external_o2)
ENDIF()

# don't install static variant, as it's only for unit testing from build directory
INSTALL(TARGETS oauth2authmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
