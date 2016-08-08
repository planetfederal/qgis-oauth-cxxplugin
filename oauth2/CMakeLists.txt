# OAuth 2 Authentication Method plugin

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

### start: when in src/auth ###
#INCLUDE_DIRECTORIES (
#  ../../core
#  ../../core/auth
#  ../../core/geometry
#)
#INCLUDE_DIRECTORIES (SYSTEM
#  ${QCA_INCLUDE_DIR}
#  ${O2_INCLUDE_DIR}
#  ${QJSON_INCLUDE_DIR}
#)
#INCLUDE_DIRECTORIES (
#  ../../gui
#  ../../gui/auth
#  ${CMAKE_CURRENT_BINARY_DIR}
#)
### end: when in src/auth ###

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

### start: when in src/auth ###
#TARGET_LINK_LIBRARIES (oauth2authmethod
#  qgis_core
#  qgis_gui
#  ${O2_LIBRARY}
#  ${QJSON_LIBRARIES}
#)
### end: when in src/auth ###

### start: when standalone ###
SET(OAUTH2_TARGET_LIBS
  ${QGIS_CORE_LIBRARY}
  ${QGIS_GUI_LIBRARY}
  ${QT_QTMAIN_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTSVG_LIBRARY}
  ${QT_QTWEBKIT_LIBRARY}
  ${QT_QTSQL_LIBRARY}
  ${O2_LIBRARY}
  ${QJSON_LIBRARIES}
)
TARGET_LINK_LIBRARIES (oauth2authmethod
  ${OAUTH2_TARGET_LIBS}
)
TARGET_LINK_LIBRARIES (oauth2authmethod_static
  ${OAUTH2_TARGET_LIBS}
)
### end: when standalone ###

INSTALL(TARGETS oauth2authmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
