import argparse
import threading
import tkinter as tk
from PIL import Image, ImageDraw, ImageTk, ImageEnhance
from wledmx import WledReceive
from input import get_parsed_args

class WindowClosedError(Exception):
    pass

class WledSimulate(threading.Thread):
    """ Simulator class for WledSend
        showing the image in a TK window instead of on the actual
        Wled display
    """


    def __init__(self, columns, rows, pixel_size):
        threading.Thread.__init__(self)
        self.stop = threading.Event()
        self.daemon = True
        self.columns = columns
        self.rows = rows
        self.pixel_size = pixel_size

    def run(self):
        # Create the background so that the window shows the correct context.
        background = Image.new("RGB", (self.columns, self.rows), "black")
        adjusted_bg = self.scale_brightness(background)
        upscaled_bg = self.upscale_image(adjusted_bg)

        # Create the Tkinter window
        root = tk.Tk()
        root.title("Wled Simulator")

        tk_image = ImageTk.PhotoImage(upscaled_bg)
        img_label = tk.Label(root, image=tk_image)
        img_label.pack()

        self.root = root
        self.img_label = img_label
        self.root.mainloop()
        self.stop.set()

    def scale_brightness(self, image, threshold=20):
        # Map brightness to the range [threshold, 255]
        def adjust_pixel(value):
            return threshold + value * (255-threshold) // 255

        # Apply the brightness adjustment
        adjusted_image = Image.new("RGB", image.size)
        for y in range(image.height):
            for x in range(image.width):
                r, g, b = image.getpixel((x, y))
                r_adj = adjust_pixel(r)
                g_adj = adjust_pixel(g)
                b_adj = adjust_pixel(b)
                adjusted_image.putpixel((x, y), (r_adj, g_adj, b_adj))

        return adjusted_image

    def upscale_image(self, image, border_size=10):
        original_width, original_height = image.size
        assert original_height == self.rows
        assert original_width == self.columns
        new_width = original_width * (self.pixel_size + border_size) + border_size
        new_height = original_height * (self.pixel_size + border_size) + border_size

        upscaled_image = Image.new("RGB", (new_width, new_height), "black")
        draw = ImageDraw.Draw(upscaled_image)

        for y in range(original_height):
            for x in range(original_width):
                color = image.getpixel((x, y))
                top_left_x = x * (self.pixel_size + border_size) + border_size
                top_left_y = y * (self.pixel_size + border_size) + border_size
                bottom_right_x = top_left_x + self.pixel_size
                bottom_right_y = top_left_y + self.pixel_size
                draw.rectangle([top_left_x, top_left_y, bottom_right_x, bottom_right_y], fill=color)

        return upscaled_image

    def send_image(self, image):
        if self.stop.is_set():
            raise WindowClosedError()
        adjusted_image = self.scale_brightness(image)
        upscaled_image = self.upscale_image(adjusted_image)
        tk_image = ImageTk.PhotoImage(upscaled_image)
        self.img_label.config(image=tk_image)
        self.img_label.image = tk_image

    def empty_image(self):
        return Image.new("RGB", (self.columns, self.rows), "black")

def runsim(host, port, columns, rows, leds_per_pixel, pixel_size):
    print("Starting test for simulate")
    import time
    simulator = WledSimulate(columns, rows, pixel_size)
    simulator.start()
    image = simulator.empty_image()
    receiver = WledReceive(host, port, columns, rows, leds_per_pixel)
    while True:
        newimage = receiver.receive_image()
        if newimage:
            image = newimage

        try:
            simulator.send_image(image)
        except WindowClosedError:
            break
    simulator.join()

if __name__ == "__main__":
    args = get_parsed_args()
    runsim(args.host, args.port, args.columns, args.rows, args.leds_per_pixel, args.pixel_size)

