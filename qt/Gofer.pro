QT      += core gui
TARGET  =  Gofer
TEMPLATE=  app
OTHER_FILES += Gofer.ico
HEADERS =	ItemSlotReceiver.h SearchResults.h Gofer.h \
		SavedSearches.h SearchState.h TextViewer.h
SOURCES =	ItemSlotReceiver.cpp SearchResults.cpp Gofer.cpp \
		SavedSearches.cpp SearchState.cpp main.cpp TextViewer.cpp
ICON =		Gofer.icns
RC_FILE =	Gofer.rc
CONFIG+=	x86_64 debug

# install
target.path = gofer
INCLUDEPATH += ..
LIBS += ../libgofer.a
POST_TARGETDEPS = ../libgofer.a
