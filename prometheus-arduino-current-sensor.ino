#include "EmonLib.h"
#include <SPI.h>
#include <Ethernet.h>

EnergyMonitor emon0;
EnergyMonitor emon1;
EnergyMonitor emon2;
EnergyMonitor emon3;
EnergyMonitor emon4;
EthernetServer server(80);

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

float last_watts0 = -1;
float last_amps0 = -1;
float last_watts1 = -1;
float last_amps1 = -1;
float last_watts2 = -1;
float last_amps2 = -1;
float last_watts3 = -1;
float last_amps3 = -1;
float last_watts4 = -1;
float last_amps4 = -1;

void listenForEthernetClients() {
	// listen for incoming clients
	EthernetClient client = server.available();
	if (client) {
		Serial.println("Got a client");
		// an http request ends with a blank line
		bool currentLineIsBlank = true;
		while (client.connected()) {
		    if (client.available()) {
                char c = client.read();
                if (c == '\n' && currentLineIsBlank) {
                    client.print(metrics_simple());
                    client.print(metrics_experiment());
                    break;
                }
                if (c == '\n') {
                  // you're starting a new line
                  currentLineIsBlank = true;
                } else if (c != '\r') {
                  // you've gotten a character on the current line
                  currentLineIsBlank = false;
                }
            }
        }
    delay(1);
    client.stop();
  }
}


String metrics_simple() {
    if (last_watts0 < 0 || last_amps0 < 0) {
        return("HTTP/1.1 400 Not Ready\n");
    }
    String message = "";
    message += "HTTP/1.1 200 OK\n";
    message += "Content-type: text/plain\n\n";
    message += "# HELP sensor_watts Estimated watts from the sensor\n";
    message += "# TYPE sensor_watts gauge\n";
    message += "sensor_watts{sensor=\"0\"} ";
    message += last_watts0;
    message += "\n";
    message += "sensor_watts{sensor=\"1\"} ";
    message += last_watts1;
    message += "\n";
    message += "# HELP sensor_amps Estimated amps from the sensor\n";
    message += "# TYPE sensor_amps gauge\n";
    message += "sensor_amps{sensor=\"0\"} ";
    message += last_amps0;
    message += "\n";
    message += "sensor_amps{sensor=\"1\"} ";
    message += last_amps1;
    message += "\n";
    return(message);
}

String metrics_experiment() {
    String message = "";
    message += "# HELP sensor_micros output of micros function\n";
    message += "# TYPE sensor_micros counter\n";
    message += "power_sensor_micros ";
    message += micros();
    message += "\n";
    return(message);
}

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

    server.begin();

    float calibration_const = 90.9;
    emon0.current(0, calibration_const);             // Current: input pin, calibration.
    emon1.current(1, calibration_const);             // Current: input pin, calibration.
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

    double Irms;
    float voltage = 120.0;
    int sample_factor = 1676;
    Irms = emon0.calcIrms(sample_factor);  // Calculate Irms only
    Serial.print(Irms*voltage);           // Apparent power
    Serial.print(" ");
    Serial.println(Irms);             // Irms
    last_watts0 = Irms*voltage;
    last_amps0 = Irms;
    
    Irms = emon1.calcIrms(sample_factor);  // Calculate Irms only
    Serial.print(Irms*voltage);           // Apparent power
    Serial.print(" ");
    Serial.println(Irms);             // Irms
    last_watts1 = Irms*voltage;
    last_amps1 = Irms;
    

    listenForEthernetClients();
}

