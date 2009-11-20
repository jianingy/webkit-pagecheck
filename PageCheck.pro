QT       +=  webkit network
SOURCES   =  PageCheck.cc
HEADERS   =  PageCheck.hh
CONFIG   +=  qt console

contains(CONFIG, static): {
  QTPLUGIN += qjpeg
  DEFINES  += STATIC_PLUGINS
}

