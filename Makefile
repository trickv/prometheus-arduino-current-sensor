SKETCH=prometheus-arduino-current-sensor.ino

upload:
	arduino --board arduino:avr:uno --port /dev/ttyACM0 --upload ${SKETCH}

verify:
	arduino --verify ${SKETCH}

serialmonitor:
	picocom /dev/ttyACM0 -b 9600
