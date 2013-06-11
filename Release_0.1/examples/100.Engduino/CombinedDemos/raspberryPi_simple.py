import serial, time
import os

engduino = serial.Serial('/dev/ttyACM0', 115200, timeout=1)

engduino.open()

try:
        while True:
#		os.system('clear')
                print "Hello what would you like to do?"
                print "1. red"
                print "2. green"
                print "3. blue"
                print "4. off"
                print "5. temp"		
                choice = raw_input("> ")

                if choice == "1" or choice == "red":
                        engduino.write('r')
                elif choice == "2" or choice == "green":
                        engduino.write('g')
                elif choice == "3" or choice == "blue":
                        engduino.write('b')
                elif choice == "4" or choice == "off":
                        engduino.write('o')
                elif choice == "5" or choice == "temp":
                        engduino.write('t')
                        temp = engduino.readline()
                        print "Temp is " + str(temp)

except KeyboardInterrupt:
        engduino.close()
