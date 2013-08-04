CONFIG += qtestlib debug
TEMPLATE = app
TARGET = 
DEFINES += private=public

# Test code
DEPENDPATH += .
INCLUDEPATH += .
SOURCES += TestThermostat.cpp

# Code to test
DEPENDPATH  += ../../FunTechHouse_Thermostat/
INCLUDEPATH += ../../FunTechHouse_Thermostat/
SOURCES += Thermostat.cpp MQTT_Logic.cpp

