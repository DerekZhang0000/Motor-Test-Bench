import serial

DESKTOP_PORT = 'COM5'
BAUD_RATE = 57600

serial_conn = serial.Serial(DESKTOP_PORT, BAUD_RATE)

while True:
    inp = input("Enter an input: ")
    serial_conn.write(inp.encode('utf-8'))
    line = serial_conn.readline().decode('utf-8').strip()
    print(line)