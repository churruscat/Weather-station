# Weather-station
# weather_station
Weather station based in multiple ESP8266 agents and a Raspberry IOT server based on Docker. 
Raspberry runs mqtt broker (mosquitto), influxdb (database), grafana (graphics visualizer) and other containers to implement the IOT server.
## MQTT agent(s)
The agent is based in an ESP8266 (a kind of Arduino with WiFi) where to connect and run sensors in modular approach, so this project can easily be used as the foundation for others.
## MQTT Server (+ other IOT functions)
Server is based on a Raspberry, and can be used for any other IOT project.
## Directories
Agent: ESP8266/arduino programs  
Server: Raspberry Pi configuration files    
in root: Documentation   
# Contact me at
churruscat@gmail.com