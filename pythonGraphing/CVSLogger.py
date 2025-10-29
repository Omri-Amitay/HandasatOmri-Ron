import serial
import time
import csv
import sys

PORT = "COM4"       # ← change to your Arduino port
BAUD = 115200       # ← match Serial.begin(...)
OUTFILE = "data.csv"

ser = serial.Serial(PORT, BAUD)
time.sleep(2)  # let Arduino reset

with open(OUTFILE, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["pulseLength", "rpm"])  # header (x,y order)

    print("Logging... will stop if Arduino sends 'Done'.")
    try:
        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if not line:
                continue

            # stop if Arduino says "Done"
            if line.lower() == "done":
                print("Arduino sent 'Done' — stopping.")
                break

            try:
                rpm_str, pulse_str = line.split(",")
                rpm = float(rpm_str)
                pulse = float(pulse_str)

                writer.writerow([pulse, rpm])
                print(f"pulse={pulse} rpm={rpm}")

            except ValueError:
                print("skipped:", line)
                continue

    except KeyboardInterrupt:
        print("Stopped manually.")

print("✅ Data logging complete.")
ser.close()
