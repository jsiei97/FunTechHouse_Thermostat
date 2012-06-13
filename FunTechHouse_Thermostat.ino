#include <SPI.h>
#include <Ethernet.h>
#include "PubSubClient.h"

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0x03 };
byte ip[]     = { 192, 168, 0, 33 };
byte server[] = { 192, 168, 0, 64 };

char project_name[]  = "FunTechHouse_Thermostat";
char topic_in[]  = "FunTechHouse/Room1/ThermostatData"; ///< data from the server
char topic_out[] = "FunTechHouse/Room1/Thermostat";     ///< data to the server

PubSubClient client(server, 1883, callback);

volatile double setpoint = 35; 
volatile int    actuator = 0;


//The relay is connected to this pin
int gpio_out  = 12;

void callback(char* topic, byte* payload, unsigned int length) 
{
    // handle message arrived
    client.publish(topic_out, "echo...");
}

void setup()
{
    analogReference(EXTERNAL); //3.3V
    pinMode(gpio_out, OUTPUT);

    Ethernet.begin(mac, ip);
    if (client.connect(project_name)) 
    {
        client.publish(topic_out, "#Hello world");
        client.subscribe(topic_in);
    }
}

void loop()
{
    int reading = analogRead(A2);
    double temperature = reading * 3.30; // 3.3V Aref 
    temperature /= 1024.0;               // ADC resolution
    temperature *= 100;                  // 10mV/C (0.01V/C)

    if(client.loop() == false)
    {
        client.connect(project_name);
    }

    //todo +- 2deg hyst.
    if(temperature < setpoint)
    {
        //Since the temperature is to low, 
        //enable the relay so we get more heat.
        digitalWrite(gpio_out, HIGH);
        actuator = 100;
    }
    else
    {
        //Since the temperature is to high, 
        //disable the relay so we turn off the heat.
        digitalWrite(gpio_out, LOW);
        actuator = 0;
    }

    char str[80];
    int temp_hel = (int)(temperature);
    int temp_del = (int)((temperature-temp_hel)*10);

    // Add setpoint to string
    snprintf(str, 80, "temperature=%d.%d ; raw=%04d ; actuator=%d", temp_hel, temp_del, reading, actuator);

    if(client.connected())
    {
        client.publish(topic_out, str);
    }

    delay(5000); 
}
