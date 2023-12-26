import configparser
import csv
import os
import serial
import subprocess
import sys
import threading
import time
import tkinter as tk
from tkinter import filedialog

"""
    Logging Setup
"""
logging = True
program_dir_path = ""
data_dir_path = ""
if getattr(sys, "frozen", False) and hasattr(sys, "_MEIPASS"):  # Program path changes if pyinstaller is used
    program_dir_path = os.path.dirname(os.path.abspath(sys.executable))
    data_dir_path = sys._MEIPASS
else:
    program_dir_path = os.path.dirname(os.path.realpath(__file__))
    data_dir_path = program_dir_path

if not os.path.exists(os.path.join(program_dir_path, "logs")):  # Creates a logs directory
    os.mkdir(os.path.join(program_dir_path, "logs"))

csv_filename = f"test_bench_data_{time.strftime('%Y-%m-%d_%H-%M-%S')}.csv"
csv_path = os.path.join(program_dir_path, "logs", csv_filename)
log_file_writer = None

def write_to_csv(item_list):
    if logging:
        with open(csv_path, 'a', newline="") as csv_file:
            log_file_writer = csv.writer(csv_file)
            log_file_writer.writerow(item_list)
write_to_csv(["Variable", "Value", "Timestamp"])

"""
    Config Setup
"""
if not os.path.exists(os.path.join(program_dir_path, ".settings")): # Creates a hidden settings directory
    os.mkdir(os.path.join(program_dir_path, ".settings"))
    subprocess.call(["attrib", "+H", os.path.join(program_dir_path, ".settings")])

config = configparser.ConfigParser()
config_path = os.path.join(program_dir_path, ".settings", "config.ini")
config.read(config_path)
if not config.has_section("Settings"):  # Creates a config file with default values if one does not exist
    config.add_section("Settings")
    config.set("Settings", "SERIAL_PORT", "COM5")
    config.set("Settings", "POLE_PAIRS", "6")
    config.set("Settings", "MIN_PWM", "127")
    config.set("Settings", "MAX_PWM", "120")
    config.set("Settings", "RESET_PWM", "150")
    with open(config_path, 'w') as configfile:
        config.write(configfile)

def write_to_config(var, val):
    config.set("Settings", var, str(val))
    with open(config_path, 'w') as configfile:
        config.write(configfile)

SERIAL_PORT = config.get("Settings", "SERIAL_PORT")
BAUD_RATE = 57600
POLE_PAIRS = int(config.get("Settings", "POLE_PAIRS"))
MIN_PWM = int(config.get("Settings", "MIN_PWM"))
MAX_PWM = int(config.get("Settings", "MAX_PWM"))
RESET_PWM = int(config.get("Settings", "RESET_PWM"))    # PWM that resets/arms the ESC
STOP_PWM = 0    # PWM that silences the ESC

serial_conn = serial.Serial(SERIAL_PORT, BAUD_RATE)

def write_to_serial(command):
    serial_conn.write(f"{command}".encode("utf-8"))

"""
    UI Setup
"""
root = tk.Tk()
root.title("JAD Test Bench UI")
root.iconbitmap(os.path.join(data_dir_path, "assets", "app-logo.ico"))
root.geometry("1060x520")

"""
    Row 0
"""
# Custom Command Input
custom_command_entry = tk.Text(root, width=65, height=4)
custom_command_entry.grid(row=0, column=0, padx=10, pady=10)

# Custom Command Input Button
def send_command():
    command = custom_command_entry.get("1.0", tk.END).strip()
    write_to_serial(command)
send_command_button = tk.Button(root, text="Enter", command=send_command)
send_command_button.grid(row=0, column=1, columnspan=2, padx=10, pady=10, sticky=tk.SW)
# Custom Command Input Button Label
send_command_label = tk.Label(root, text="Send Custom Command")
send_command_label.grid(row=0, column=1, columnspan=2, padx=10, pady=10, sticky=tk.NW)

# Load PWM Program Button
def load_pwm_program():
    file_path = filedialog.askopenfilename(title="Select a File", filetypes=[("CSV File", "*.csv")])

    def process_pwm_program():
        with open(file_path, 'r') as csv_file:
            write_to_serial(f"pwm={RESET_PWM}")
            time.sleep(1)
            if not logging:
                toggle_logging()
            time.sleep(1)

            csv_reader = csv.reader(csv_file)
            for value_list in csv_reader:
                if value_list == [] or not value_list[0].isnumeric():
                    continue
                pwm, duration = value_list
                write_to_serial(f"pwm={pwm}")
                time.sleep(float(duration))

        stop_esc()
        if logging:
            toggle_logging()

    threading.Thread(target=process_pwm_program).start()    # Runs an anonymous thread so that time.sleep does not freeze the UI

load_pwm_program_button = tk.Button(root, text="Load", command=load_pwm_program)
load_pwm_program_button.grid(row=0, column=5, columnspan=2, padx=10, pady=10, sticky=tk.SE)
# Load PWM Program Button Label
load_pwm_program_label = tk.Label(root, text="Load PWM Program")
load_pwm_program_label.grid(row=0, column=5, columnspan=2, padx=10, pady=10, sticky=tk.NE)

"""
    Row 1
"""
# Terminal Output
var_dict = {"lc" : "LOAD CELL",
            "rpm" : "RPM",
            "log_start" : "LOG START",
            "log_stop" : "LOG STOP",}
def update_terminal():
    while True:
        line = ""
        try:
            line = serial_conn.readline().decode("utf-8").strip()
        except:
            pass
        if logging or "log_start" in line or "log_stop" in line:
            if len(line) == 0 or '=' not in line:
                continue
            if line[0] == '<':
                line = line[1:]

            var, val = line.split('=')
            if var in var_dict:
                var = var_dict[var]
            else:
                var = var.upper()

            if "," in val:  # The ',' character is only used in variables with a timestamp, which are logged
                write_to_csv([var] + val.split(','))
                val = val.replace(',', " : ")

            line = f"[{var}] {val}"
            terminal_text.configure(state=tk.NORMAL)
            terminal_text.insert(tk.END, f"{line}\n")
            terminal_text.configure(state=tk.DISABLED)
            terminal_text.see(tk.END)
terminal_text = tk.Text(root, width=65, state=tk.DISABLED, height=20)
terminal_text.grid(row=1, column=0, padx=10, pady=10)

# ESC PWM Reset Button
def reset_pwm():
    update_reset_pwm()
    write_to_serial(f"pwm={RESET_PWM}")
reset_pwm_button = tk.Button(root, text="Reset", command=reset_pwm)
reset_pwm_button.grid(row=1, column=1, padx=10, pady=10, sticky=tk.SW)

# Stop ESC Button
def stop_esc():
    write_to_serial(f"pwm={STOP_PWM}")
stop_esc_button = tk.Button(root, text="STOP", command=stop_esc, fg="white", bg="red")
stop_esc_button.grid(row=1, column=1, padx=10, pady=10, sticky=tk.W)

# JAD Logo
logo_path = os.path.join(data_dir_path, "assets", "JAD-logo.png")
logo = tk.PhotoImage(file=logo_path)
logo = logo.subsample(5, 5)
logo_label = tk.Label(root, image=logo)
logo_label.grid(row=1, column=2, columnspan=5, padx=10, pady=10)

"""
    Row 2
"""
# ESC PWM Slider
def pwm_slider_change(event=None):
    write_to_serial(f"pwm={pwm_slider.get()}")
pwm_slider = tk.Scale(root, from_=MIN_PWM, to=MAX_PWM, orient=tk.HORIZONTAL, length=400, sliderrelief=tk.RIDGE)
pwm_slider.grid(row=2, column=0, padx=10, pady=10, sticky=tk.S)
pwm_slider.bind("<ButtonRelease>", pwm_slider_change)
pwm_slider.set(MIN_PWM)
# ESC PWM Label
pwm_slider_label = tk.Label(root, text="PWM")
pwm_slider_label.grid(row=2, column=0, padx=10, pady=10, sticky=tk.SW)

# ESC PWM Reset Entry
def update_reset_pwm(event=None):
    global RESET_PWM
    try:
        RESET_PWM = int(reset_pwm_entry.get())
        write_to_config("RESET_PWM", RESET_PWM)
    except:
        pass
reset_pwm_entry = tk.Entry(root, width=10)
reset_pwm_entry.grid(row=2, column=1, padx=10, pady=10, sticky=tk.S)
reset_pwm_entry.insert(0, RESET_PWM)
reset_pwm_entry.bind("<Return>", update_reset_pwm)
# ESC PWM Reset Label
reset_pwm_label = tk.Label(root, text="Reset PWM")
reset_pwm_label.grid(row=2, column=1, padx=10, pady=10, sticky=tk.N)

# Min/Max PWM Entries
def update_pwm_slider_range(event=None):
    global MIN_PWM, MAX_PWM
    try:
        MIN_PWM = int(min_pwm_entry.get())
        MAX_PWM = int(max_pwm_entry.get())
        pwm_slider.config(from_=MIN_PWM, to=MAX_PWM)
        write_to_config("MIN_PWM", MIN_PWM)
        write_to_config("MAX_PWM", MAX_PWM)
    except:
        pass
# Min Slider Value Entry
min_pwm_entry = tk.Entry(root, width=10)
min_pwm_entry.grid(row=2, column=2, padx=10, pady=10, sticky=tk.S)
min_pwm_entry.insert(0, MIN_PWM)
min_pwm_entry.bind("<Return>", update_pwm_slider_range)
# Min Slider Value Label
min_pwm_label = tk.Label(root, text="Min PWM")
min_pwm_label.grid(row=2, column=2, padx=10, pady=10, sticky=tk.N)

# Max Slider Value Entry
max_pwm_entry = tk.Entry(root, width=10)
max_pwm_entry.grid(row=2, column=3, padx=10, pady=10, sticky=tk.S)
max_pwm_entry.insert(0, MAX_PWM)
max_pwm_entry.bind("<Return>", update_pwm_slider_range)
# Max Slider Value Label
max_pwm_label = tk.Label(root, text="Max PWM")
max_pwm_label.grid(row=2, column=3, padx=10, pady=10, sticky=tk.N)

# Toggle Logging Button
def update_logging_button_color():
    global logging
    if logging:
        toggle_logging_button.config(bg="green")
    else:
        toggle_logging_button.config(bg="red")
def toggle_logging():
    global logging
    if logging:
        write_to_serial(f"log_stop=0")
        logging = False
    else:
        write_to_serial(f"log_start=0")
        logging = True
    update_logging_button_color()
toggle_logging_button = tk.Button(root, text="Logging", command=toggle_logging, fg="white", bg="green")
toggle_logging_button.grid(row=2, column=4, padx=10, pady=10, sticky=tk.S)

# Pole Pairs Entry
def update_pole_pairs(event=None):
    global POLE_PAIRS
    try:
        POLE_PAIRS = int(pole_pairs_entry.get())
        write_to_serial(f"pole_pairs={POLE_PAIRS}")
        write_to_config("POLE_PAIRS", POLE_PAIRS)
    except:
        pass
pole_pairs_entry = tk.Entry(root, width=10)
pole_pairs_entry.grid(row=2, column=5, padx=10, pady=10, sticky=tk.S)
pole_pairs_entry.insert(0, POLE_PAIRS)
pole_pairs_entry.bind("<Return>", update_pole_pairs)
# Pole Pairs Label
pole_pairs_label = tk.Label(root, text="Pole Pairs")
pole_pairs_label.grid(row=2, column=5, padx=10, pady=10, sticky=tk.N)

# Serial Port Entry
def update_serial_port(event=None):
    global SERIAL_PORT
    try:
        SERIAL_PORT = serial_port_entry.get()
        serial_conn.port = SERIAL_PORT
        serial_conn.close()
        serial_conn.open()
        write_to_config("SERIAL_PORT", SERIAL_PORT)
    except:
        pass
serial_port_entry = tk.Entry(root, width=10)
serial_port_entry.grid(row=2, column=6, padx=10, pady=10, sticky=tk.S)
serial_port_entry.insert(0, SERIAL_PORT)
serial_port_entry.bind("<Return>", update_serial_port)
# Serial Port Label
serial_port_label = tk.Label(root, text="Serial Port")
serial_port_label.grid(row=2, column=6, padx=10, pady=10, sticky=tk.N)

terminal_thread = threading.Thread(target=update_terminal, daemon=True)
terminal_thread.start()

root.after(2000, update_pole_pairs) # ESC does not automatically update pole pairs from config settings

root.mainloop()