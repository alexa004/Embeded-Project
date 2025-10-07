import serial

import openpyxl

from datetime import datetime

 

# Set your Arduino port here!

PORT = '/dev/tty.usbmodem1301'  # your port

BAUD = 9600

FILENAME = 'Vehicle_Log.xlsx'

 

# Setup serial connection

ser = serial.Serial(PORT, BAUD)

print(f"Connected to {PORT} at {BAUD} baud")

 

# Create or load workbook

try:

   wb = openpyxl.load_workbook(FILENAME)

   ws = wb.active

except FileNotFoundError:

   wb = openpyxl.Workbook()

   ws = wb.active

   ws.append(['Timestamp', 'UID', 'Vehicle Type', 'Action', 'Slot'])

 

# Read from serial

while True:

   line = ser.readline().decode().strip()

   print(line)

 

   if 'UID:' in line:

       uid = line.split(':')[1].strip()

   elif 'Detected:' in line:

       vehicle_type = line.split(':')[1].strip()

   elif 'Entry' in line or 'Exit' in line:

       parts = line.split()

       action = parts[0]

       slot = parts[-1] if 'Slot' in parts else 'Unknown'

 

       timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

       ws.append([timestamp, uid, vehicle_type, action, slot])

       wb.save(FILENAME)
