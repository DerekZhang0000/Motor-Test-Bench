import serial, tkinter as tk
from tkinter import messagebox

DESKTOP_PORT = 'COM5'
BAUD_RATE = 57600
RESET_PWM = 150

serial_conn = serial.Serial(DESKTOP_PORT, BAUD_RATE)

def on_submit():
    text_value1 = entry1.get("1.0", tk.END).strip()
    serial_conn.write(f"{text_value1}".encode('utf-8'))
    # messagebox.showinfo("Submission", f"Text: {text_value1}\n")

def on_reset():
    serial_conn.write(f"pwm={RESET_PWM}".encode('utf-8'))

def on_slider_change(value):
    serial_conn.write(f"pwm={value}".encode('utf-8'))

# Create the main window
root = tk.Tk()
root.title("Tkinter Application")

# Set window size
root.geometry("1000x600")

# Create and place widgets
label1 = tk.Label(root, text="Text Box 1:")
label1.grid(row=0, column=0, padx=10, pady=10, sticky=tk.W)

entry1 = tk.Text(root, width=60, height=20)
entry1.grid(row=0, column=1, padx=10, pady=10)

label2 = tk.Label(root, text="Text Box 2 (Read-Only):")
label2.grid(row=1, column=0, padx=10, pady=10, sticky=tk.W)

entry2 = tk.Text(root, width=60, state=tk.DISABLED, height=10)
entry2.grid(row=1, column=1, padx=10, pady=10)

label3 = tk.Label(root, text="Slider:")
label3.grid(row=2, column=0, padx=10, pady=10, sticky=tk.W)

slider = tk.Scale(root, from_=120, to=128, orient=tk.HORIZONTAL, command=on_slider_change)
slider.grid(row=2, column=1, padx=10, pady=10)

button_submit = tk.Button(root, text="Submit", command=on_submit)
button_submit.grid(row=0, column=2, columnspan=2, pady=10)

button_reset = tk.Button(root, text="Reset", command=on_reset)
button_reset.grid(row=2, column=2, columnspan=2, pady=10)

# Run the Tkinter event loop
root.mainloop()
