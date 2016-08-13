# OAuth 2 Authentication Method plugin

########################################################
# Packages
if(WITH_DESKTOP AND ENABLE_QT5)
  #
elseif(WITH_DESKTOP)
  set(QT_USE_QTSCRIPT 1)
  include(${QT_USE_FILE})
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(QJSON REQUIRED)
find_package(QCA REQUIRED)
find_package(O2 REQUIRED)

SET(OAUTH2_SRCS
  qgso2.cpp
  qgsauthoauth2config.cpp
  qgsauthoauth2method.cpp
  qgspasswordlineedit.cpp
  qgsauthoauth2edit.cpp
)

SET(OAUTH2_HDRS
  qgso2.h
  qgsauthoauth2config.h
  qgsauthoauth2method.h
  qgspasswordlineedit.h
  qgsauthoauth2edit.h
)

SET(OAUTH2_MOC_HDRS
  qgso2.h
  qgsauthoauth2config.h
  qgsauthoauth2method.h
  qgspasswordlineedit.h
  qgsauthoauth2edit.h
)

SET(OAUTH2_UIS qgsauthoauth2edit.ui)

SET(OAUTH2_RCCS  oauth2_resources.qrc)

IF(WITH_DESKTOP)
  ### start: when in src/auth ###
  INCLUDE_DIRECTORIES (
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/core/auth
    ${CMAKE_SOURCE_DIR}/src/core/geometry
    ${CMAKE_SOURCE_DIR}/src/auth/oauth2
    ${CMAKE_SOURCE_DIR}/src/gui
    ${CMAKE_SOURCE_DIR}/src/gui/auth
    ${CMAKE_CURRENT_SOURCE_DIR}
  )
  INCLUDE_DIRECTORIES (SYSTEM
    ${QCA_INCLUDE_DIR}
    ${O2_INCLUDE_DIR}
    ${QJSON_INCLUDE_DIR}
  )
  INCLUDE_DIRECTORIES (
    ${CMAKE_BINARY_DIR}/src/ui
    ${CMAKE_BINARY_DIR}/src/ui/auth
    ${CMAKE_BINARY_DIR}/src/auth/oauth2
    ${CMAKE_CURRENT_BINARY_DIR}
  )
  ### end: when in src/auth ###
ELSE(WITH_DESKTOP)
  ### start: when standalone ###
  INCLUDE_DIRECTORIES (SYSTEM
    ${QT_INCLUDE_DIR}
    ${QGIS_INCLUDE_DIR}
    ${QCA_INCLUDE_DIR}
    ${QJSON_INCLUDE_DIR}
  )
  INCLUDE_DIRECTORIES (
    ${O2_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
  )
  ### end: when standalone ###
ENDIF(WITH_DESKTOP)


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

ADD_LIBRARY (oauth2authmethod_static STATIC
  ${OAUTH2_SRCS}
  ${OAUTH2_HDRS}
  ${OAUTH2_MOC_SRCS}
  ${OAUTH2_RCC_SRCS}
  ${OAUTH2_UIS_H}
)

IF(WITH_DESKTOP)
  # in QGIS source tree
  SET(OAUTH2_TARGET_LIBS
    qgis_core
    qgis_gui
    ${QT_QTSCRIPT_LIBRARY}
    ${O2_LIBRARY}
    ${QJSON_LIBRARIES}
  )
ELSE(WITH_DESKTOP)
  # outside QGIS source tree
  SET(OAUTH2_TARGET_LIBS
    ${QGIS_CORE_LIBRARY}
    ${QGIS_GUI_LIBRARY}
    ${QT_QTMAIN_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTSCRIPT_LIBRARY}
    ${QT_QTSVG_LIBRARY}
    ${O2_LIBRARY}
    ${QJSON_LIBRARIES}
  )
ENDIF(WITH_DESKTOP)

TARGET_LINK_LIBRARIES (oauth2authmethod
  ${OAUTH2_TARGET_LIBS}
)
TARGET_LINK_LIBRARIES (oauth2authmethod_static
  ${OAUTH2_TARGET_LIBS}
)

# don't install static variant, as it's only for unit testing from build directory
INSTALL(TARGETS oauth2authmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
