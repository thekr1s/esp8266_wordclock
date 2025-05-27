#!/bin/python3
import argparse
import socket
import requests

class Woordklok:
    """
    REST interface of the woordklok
    """
    def __init__(self, ip):
        self.ip = ip
        self.urlStart = "http://" + ip
        
    def start_update(self):
        endpoint = "/wificfg/hwcfg.html"
        url = self.urlStart + endpoint
        data = {'cl_command': 'OtaUpdate'}
        try:
            x = requests.post(url, data = data)
            #print(x.text)
        finally:
            print("Firmware update is started....")

    def start_realtime_udp(self):
        endpoint = "/wificfg/controller.html"
        url = self.urlStart + endpoint
        data = {'cl_command': 'UDP_Realtime'}
        try:
            x = requests.post(url, data = data)
            #print(x.text)
        finally:
            print("Realtime udp started")

def generate_LUT(ldr_min=2, ldr_max=900, led_min=2, led_max=220, gamma=2.2, steps=16):
    """
    Generate a lookup table (LUT) for the LED values based on LDR input.
    The LUT is generated using a gamma correction formula.
    """
    import numpy as np

    # Generate LDR input values
    ldr_values = np.linspace(ldr_min, ldr_max, steps)

    # Normalize and apply gamma correction
    ldr_normalized = (ldr_values - ldr_min) / (ldr_max - ldr_min)
    led_values = led_min + (led_max - led_min) * np.power(1 - ldr_normalized, gamma)
    led_values = np.round(led_values).astype(int)

    # Print formatted output
    print("LDR values:")
    print(",".join(str(int(v)) for v in ldr_values) + ",")
    print("LED values:")
    print(",".join(str(v) for v in led_values) + ",")

def get_parsed_args():
    # Make parser object
    p = argparse.ArgumentParser(description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    
    p.add_argument("--ip", default="0.0.0.0",
                   help="IP-address of the woordklok")
    p.add_argument("--update", action='store_true',
                   help="Request the woordklok to fetch new updates")
    p.add_argument("--realtime_udp",  action='store_true',
                   help="Set woordklok in realtime udp mode")
    return(p.parse_args())

if __name__ == '__main__':
    args = get_parsed_args()

    api = Woordklok(args.ip)
    if args.update:
        api.start_update()
    if args.realtime_udp:
        api.start_realtime_udp()
    #generate_LUT()