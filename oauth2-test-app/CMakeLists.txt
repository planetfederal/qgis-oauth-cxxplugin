########################################################
# Files

set(OAUTH2_APP_SRCS
  testwidget.cpp
  browser.cpp
  main.cpp
)

set(OAUTH2_APP_UIS
  browser.ui
  testwidget.ui
)

set(OAUTH2_APP_MOC_HDRS
  browser.h
  testwidget.h
)

########################################################
# Build

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

QT4_WRAP_UI(OAUTH2_APP_UIS_H  ${OAUTH2_APP_UIS})

QT4_WRAP_CPP(OAUTH2_APP_MOC_SRCS  ${OAUTH2_APP_MOC_HDRS})

SET(IMAGE_RCCS images/images.qrc)
QT4_ADD_RESOURCES(OAUTH2_IMAGE_RCC_SRCS ${IMAGE_RCCS})

include_directories(SYSTEM
  ${QT_INCLUDE_DIR}
  ${QGIS_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
)
include_directories(
  ${O2_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/oauth2
  ${CMAKE_BINARY_DIR}/oauth2
)

if(NOT MSVC)
  add_definitions("-UAPP_EXPORT")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
  endif()
endif(NOT MSVC)
add_definitions("\"-DAPP_EXPORT=${DLLEXPORT}\"")

add_executable(oauth2testapp MACOSX_BUNDLE WIN32
  ${OAUTH2_APP_SRCS}
  ${OAUTH2_APP_MOC_SRCS}
  ${OAUTH2_APP_UIS_H}
  ${OAUTH2_IMAGE_RCC_SRCS}
)

target_link_libraries(oauth2testapp
  ${QGIS_CORE_LIBRARY}
  ${QGIS_GUI_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${QT_QTWEBKIT_LIBRARY}
  ${QT_QTMAIN_LIBRARY}
  ${O2_LIBRARY}
)

if(APPLE)
  add_definitions(-DQT_NO_CAST_TO_ASCII)
  target_link_libraries(oauth2testapp ${APP_SERVICES_LIBRARY})
  set_target_properties(oauth2testapp PROPERTIES
    OUTPUT_NAME "OAuth2-Test"
  )
endif(APPLE)

########################################################
# Install

install(TARGETS oauth2testapp
  BUNDLE DESTINATION bin
  RUNTIME DESTINATION bin
)
