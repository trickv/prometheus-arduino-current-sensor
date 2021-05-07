#include "EmonLib.h"
#include <SPI.h>
#include <Ethernet.h>

EnergyMonitor emon0;
EnergyMonitor emon1;
EthernetServer server(80);

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

unsigned long loop_count = 0;
unsigned long previous_millis = -1;
unsigned long last_loop_millis = -1;
float total_amp_milliseconds0 = 0;
double total_amp_hours0 = 0;
float last_watts0 = -1;
float last_amps0 = -1;
unsigned long last_time0 = -1;
float total_amp_milliseconds1 = 0;
double total_amp_hours1 = 0;
float last_watts1 = -1;
float last_amps1 = -1;
unsigned long last_time1 = -1;
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
//                    client.print(metrics_experiment());
                    client.print(metrics_experiment_2());
                    client.print(metrics_cumulative_amp_milliseconds());
                    client.print(metrics_cumulative_amp_hours());
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
    //message += "# HELP sensor_watts Estimated watts from the sensor\n";
    /*message += "# TYPE sensor_watts gauge\n";
    message += "sensor_watts{sensor=\"0\"} ";
    message += last_watts0;
    message += "\n";
    message += "sensor_watts{sensor=\"1\"} ";
    message += last_watts1;
    message += "\n";*/
    //message += "# HELP sensor_amps Estimated amps from the sensor\n";
    message += "# TYPE sensor_amps gauge\n";
    message += "sensor_amps{sensor=\"0\"} ";
    message += last_amps0;
    message += "\n";
    message += "sensor_amps{sensor=\"1\"} ";
    message += last_amps1;
    message += "\n";
    return(message);
}

String make_metric_output(String name, String flags, String type, String value) {
    String message = "";
    //message += "# HELP " + name + " x\n";
    message += "# TYPE " + name + " " + type + "\n";
    message += name;
    if (flags.length() > 0) {
        message += "{" + flags + "}";
    }
    message += " ";
    message += value;
    message += "\n";
    return(message);
}

String metrics_experiment() {
    String message = "";
    //message += "# HELP sensor_micros output of micros function\n";
    //m//essage += "# TYPE sensor_micros counter\n";
   // message += "power_sensor_micros ";
    //message += micros();
    //message += "\n";

    message += "# HELP power_sensor_loop_count debug\n";
    message += "# TYPE power_sensor_loop_count counter\n";
    message += "power_sensor_loop_count ";
    message += loop_count;
    message += "\n";
    return(message);
}

String metrics_experiment_2() {
    String message = "";
    message += make_metric_output("power_sensor_sense_time", "sensor=\"0\"", "gauge", String(last_time0));
    message += make_metric_output("power_sensor_sense_time", "sensor=\"1\"", "gauge", String(last_time1));
    message += make_metric_output("power_sensor_loop_time", "", "gauge", String(last_loop_millis));
    return(message);
}

String metrics_cumulative_amp_milliseconds() {
    String message = "";
    message += make_metric_output("power_sensor_amp_milliseconds", "sensor=\"0\"", "gauge", String(total_amp_milliseconds0));
    message += make_metric_output("power_sensor_amp_milliseconds", "sensor=\"1\"", "gauge", String(total_amp_milliseconds1));
    return(message);
}

String metrics_cumulative_amp_hours() {
    String message = "";
    message += make_metric_output("power_sensor_amp_hours", "sensor=\"0\"", "gauge", String(total_amp_hours0, 8));
    message += make_metric_output("power_sensor_amp_hours", "sensor=\"1\"", "gauge", String(total_amp_hours1, 8));
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

    unsigned long now = millis();
    last_loop_millis = now - previous_millis;
    previous_millis = now;
    loop_count += 1;

    double Irms;
    float voltage = 120.0;
    int sample_factor = 1676;

    unsigned long start_time0 = millis();
    Irms = emon0.calcIrms(sample_factor);  // Calculate Irms only
    Serial.print(Irms*voltage);           // Apparent power
    Serial.print(" ");
    Serial.println(Irms);             // Irms
    last_watts0 = Irms*voltage;
    last_amps0 = Irms;
    last_time0 = millis() - start_time0;
    total_amp_milliseconds0 += Irms / last_loop_millis;
    total_amp_hours0 += Irms * ((last_loop_millis / (float) 1000) / (float) 3600);
    
    unsigned long start_time1 = millis();
    Irms = emon1.calcIrms(sample_factor);  // Calculate Irms only
    Serial.print(Irms*voltage);           // Apparent power
    Serial.print(" ");
    Serial.println(Irms);             // Irms
    last_watts1 = Irms*voltage;
    last_amps1 = Irms;
    last_time1 = millis() - start_time1;
    total_amp_milliseconds1 += Irms / last_loop_millis;
    total_amp_hours1 += Irms * ((last_loop_millis / (float) 1000) / (float) 3600);
    

    listenForEthernetClients();
}

