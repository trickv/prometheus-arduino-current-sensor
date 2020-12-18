#include "EmonLib.h"
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoOTA.h>

EnergyMonitor emon1;

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

void setup()
{
    Serial.begin(9600);

    if (Ethernet.begin(mac) == 0) {
        Serial.println("Failed to configure Ethernet using DHCP");
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
            Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
        } else if (Ethernet.linkStatus() == LinkOFF) {
            Serial.println("Ethernet cable is not connected.");
        }
    }

    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());

    emon1.current(1, 111.1);             // Current: input pin, calibration.
}

void loop()
{
    switch (Ethernet.maintain()) {
        case 1: // renew failed
            Serial.println("Ethernet error: renew failed");
            break;
        case 2: // renew success
            Serial.println("Ethernet OK: DHCP renew success");
            Serial.print("My IP address: ");
            Serial.println(Ethernet.localIP());
            break;
        case 3: // rebind fail
            Serial.println("Ethernet error: rebind fail");
            break;
        case 4: // rebind success
            Serial.println("Ethernet OK: rebind success");
            Serial.print("My IP address: ");
            Serial.println(Ethernet.localIP());
            break;
        default: // nothing happened with the ethernet controller
            break;
        }

    double Irms = emon1.calcIrms(1480);  // Calculate Irms only
    Serial.print(Irms*230.0);           // Apparent power
    Serial.print(" ");
    Serial.println(Irms);             // Irms
}

