CONFIG += qtestlib debug
TEMPLATE = app
TARGET = 
DEFINES += private=public

# Test code
DEPENDPATH += .
INCLUDEPATH += .
SOURCES += TestLVTS.cpp

# Code to test
DEPENDPATH  += ../../FunTechHouse_Thermostat/
INCLUDEPATH += ../../FunTechHouse_Thermostat/
SOURCES += LVTS.cpp

