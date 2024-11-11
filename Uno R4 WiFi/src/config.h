#ifndef _CONFIG_H
#define _CONFIG_H

#include <Arduino.h>

// Fill in the hostname of your IoT broker
#define SECRET_BROKER "demo.thingsboard.io"
#define MQTT_PORT 1883 

#define SSID_WIFI ""
#define PASSWORD_WIFI ""

#define TOKEN "" 
#define MQTT_USERNAME "" 
#define MQTT_PASSWORD "" 


#define PUBLISH_TOPIC "v1/devices/me/telemetry"
#define SUBSCRIBE_TOPIC "v1/devices/me/rpc/request/+"


#endif /* _CONFIG_H */

