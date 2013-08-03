CONFIG += qtestlib debug
TEMPLATE = app
TARGET = 
DEFINES += private=public

# Test code
DEPENDPATH += .
INCLUDEPATH += .
SOURCES += TestStringHelp.cpp

# Code to test
DEPENDPATH  += ../../FunTechHouse_Thermostat/
INCLUDEPATH += ../../FunTechHouse_Thermostat/
SOURCES += StringHelp.cpp

