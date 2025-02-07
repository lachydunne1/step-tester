import serial 
import struct
import argparse
import signal
import csv
import sys
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import pandas as pd
import re

"""_summary_
How It Works:

    Command-Line Argument --uart
        If --uart is passed, it enables serial communication.
        Otherwise, it only visualizes data.

    User Input
        Takes an integer input (1 to enable CONTROL_GPIO, which begins sampling
        task) and sends it via UART if enabled.

    Listening for Incoming Data
        Parses data formatted as "os_value0, os_value1, os_value2, os_value3".
        Stores received data in a list.

    Real-Time Plotting
        Uses matplotlib.animation to update the plot live.

    Handling Ctrl+C
        Saves all collected data to unit_step_data.csv.
        Sends 0 over UART before exiting.
        
    Note: User must setup the uart over USB by specifying the address on line 150.
"""

DATA_FORMAT = 'i' #int format to send over uart
CSV_FILE = "unit_step_data.csv" #SAVE DATA TO CSV
n =4 #total values read from UART
monitor0 = "value0"
monitor1 = "value1"
monitor2 = "value2"
monitor3 = "value3"

class SerialWrapper:
    
    def __init__(self, device, baud_rate=115200):
        # Initialize the serial port
        self.ser = serial.Serial(device, baud_rate, timeout=1)
        print(f"Connected to {device} at {baud_rate} baud rate.")
    
    def send_data(self, data1):
        packed_data = struct.pack(DATA_FORMAT, data1)
        self.ser.write(packed_data)  
        
    def read_data(self):
        """ brief: read and parse data over uart
        """
        try: 
            line = self.ser.readline().decode().strip()
            clean_line = clean_uart_line(line)
            line = self.ser.readline().decode().strip()
            if clean_line:
                values = list(map(float, clean_line.split(',')))
                if len(values) == n:
                    return values
            else: 
                print("Stuck at line cleaning")
        except Exception as e:
            print(f"Error reading UART data: {e}")
        
        return None
    

def signal_handler(sig, frame):
    """Handles Ctrl+C and sends a '0' over UART before exiting."""
    if uart:
        print("\nExiting: Sending 0 over UART.")
        uart.send_data(0)
    if data_buffer:
        # Save collected data to CSV
        df = pd.DataFrame(data_buffer, columns=[monitor0, monitor1, monitor2, monitor3])
        df.to_csv(CSV_FILE, index=False)
        print(f"Data saved to {CSV_FILE}")
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)  # Handle Ctrl+C

def animate(i):
    """Updates the live plot with incoming UART data."""
    global data_buffer

    if uart:
        values = uart.read_data()
        if values:
            data_buffer.append(values)
            ax.clear()
            ax.plot([row[0] for row in data_buffer], label=monitor0)
            ax.plot([row[1] for row in data_buffer], label=monitor1)
            ax.plot([row[2] for row in data_buffer], label=monitor2)
            ax.plot([row[3] for row in data_buffer], label=monitor3)
            #ax.plot([row[4] for row in data_buffer], label=monitor4)
            #ax.plot([row[5] for row in data_buffer], label=monitor5)
            ax.legend()
            ax.set_title("Live UART Data")
            ax.set_xlabel("Samples")
            ax.set_ylabel("Values")
            
            

def clean_uart_line(line):
    """Remove ANSI escape codes and filter out ESP_LOGI debug messages."""
    # Remove ANSI escape sequences
    line = re.sub(r'\x1b\[[0-9;]*m', '', line)
    
    # Extract only valid numeric data
    # Regex patterns for different 'n' values
    patterns = {
        1: r'([-+]?[0-9]*\.?[0-9]+)',  # Single value
        2: r'([-+]?[0-9]*\.?[0-9]+,[-+]?[0-9]*\.?[0-9]+)',
        3: r'([-+]?[0-9]*\.?[0-9]+(?:,[-+]?[0-9]*\.?[0-9]*){2})',
        4: r'([-+]?[0-9]*\.?[0-9]+(?:,[-+]?[0-9]*\.?[0-9]*){3})',
        5: r'([-+]?[0-9]*\.?[0-9]+(?:,[-+]?[0-9]*\.?[0-9]*){4})',
        6: r'([-+]?[0-9]*\.?[0-9]+(?:,[-+]?[0-9]*\.?[0-9]*){5})'
    } #re is dense, use chatgpt to generate extra patterns

    try:
        pattern = patterns.get(n)
    except IndexError: 
        print(f"N is configured to : {n}, out of re statement range.")
        return None
    
    match = re.search(pattern, line)
    if match:
        return match.group(1)  # Extract valid float values
    return None
    
uart = None
data_buffer = []
data =-1


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="UART DataLogger with Matplotlib Visualisation")
    parser.add_argument("--uart", action="store_true", help = "Enable UART transmission for UNIT_STEP")
    
    args = parser.parse_args()
    
    if args.uart:
        uart = SerialWrapper('/dev/ttyUSB0')

    # Ask for user input and send over UART if enabled
    data = int(input("Input: "))   
    if args.uart:
        uart.send_data(data)

    # Set up Matplotlib plot
    fig, ax = plt.subplots()
    ani = animation.FuncAnimation(fig, animate, interval=250, frames=1000)  # Update every 250ms

    plt.show()