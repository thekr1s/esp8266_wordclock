#!/bin/python3
from PIL import Image
from io import BytesIO
import matplotlib.pyplot as plt
import numpy as np
from copy import deepcopy
import time
from woordklok_api import Woordklok

host = ('192.168.2.87', 21324)

def test_send_image(api):   
    image = Image.new('RGB', (13, 13), 'Green')  # Default to a white background
    center_x, center_y = 0, 0  # Center pixel coordinates for a 13x13 image
    image.putpixel((center_x, center_y), (255, 0, 0))  # Set center pixel to red
    image.putpixel((0, 1), (255, 0, 0))  # Set center pixel to red
    image.putpixel((1, 0), (255, 0, 0))  # Set center pixel to red
    image.putpixel((1, 1), (255, 0, 0))  # Set center pixel to red
    #image.show()

    while (True):
        api.send_image(image)
        time.sleep(1)

if __name__ == '__main__':
    api = Woordklok(host)
    api.start_realtime_udp()
    test_send_image(api)

