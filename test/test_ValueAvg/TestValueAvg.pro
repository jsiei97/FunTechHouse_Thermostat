CONFIG += qtestlib debug
TEMPLATE = app
TARGET = 
DEFINES += private=public

# Test code
DEPENDPATH += .
INCLUDEPATH += .
SOURCES += TestValueAvg.cpp

# Code to test
DEPENDPATH  += ../../FunTechHouse_Thermostat/
INCLUDEPATH += ../../FunTechHouse_Thermostat/
SOURCES += ValueAvg.cpp

