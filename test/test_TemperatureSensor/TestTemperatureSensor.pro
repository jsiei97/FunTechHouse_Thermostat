CONFIG += qtestlib debug
TEMPLATE = app
TARGET = 
DEFINES += private=public

# Test code
DEPENDPATH += .
INCLUDEPATH += .
SOURCES += TestTemperatureSensor.cpp

# Code to test
DEPENDPATH  += ../../FunTechHouse_Thermostat/
INCLUDEPATH += ../../FunTechHouse_Thermostat/
SOURCES += TemperatureSensor.cpp Sensor.cpp MQTT_Logic.cpp StringHelp.cpp

