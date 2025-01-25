from PIL import Image
from io import BytesIO
import matplotlib.pyplot as plt
import numpy as np
from copy import deepcopy
import time
from input import get_parsed_args
from wledmx import WledSend

def test_send_image(api, columns, rows):   
    image = Image.new('RGB', (columns, rows), 'Green')  # Default to a white background
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
    args = get_parsed_args()
    wled = WledSend((args.host, args.port), args.columns, args.rows, args.leds_per_pixel)
    test_send_image(wled, args.columns, args.rows)
