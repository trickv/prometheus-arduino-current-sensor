SKETCH=prometheus-arduino-current-sensor.ino

upload:
	killall picocom || true
	arduino --board arduino:avr:uno --port /dev/ttyACM0 --upload ${SKETCH}

verify:
	arduino --verify ${SKETCH}

serialmonitor:
	picocom /dev/ttyACM0 -b 9600

fetch:
	curl 172.16.17.60
