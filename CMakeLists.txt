project(PicasaEngine)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
)

set(picasa_engine_SRCS
picasa.cpp
picasainterface.cpp
)

kde4_add_plugin(plasma_engine_picasa ${picasa_engine_SRCS})
target_link_libraries(plasma_engine_picasa ${KDE4_KDECORE_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY})

install(TARGETS plasma_engine_picasa DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-picasa.desktop DESTINATION ${SERVICES_INSTALL_DIR})
