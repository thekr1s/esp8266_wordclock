#!/usr/bin/env python3
import socket

class WledSend:
    """
    Sends data to WLED devices using UDP protocol.
    """

    WLED_DRGB = 2  # Protocol selection
    WLED_TIMEOUT = 5  # Timeout in seconds
    ROWS_REVERSED = False

    def __init__(self, host, columns=5, rows=4, leds_per_pixel=20):
        self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.host = host
        self.columns = columns
        self.rows = rows
        self.leds_per_pixel = leds_per_pixel
        self.row_directions = [False] * self.rows

        self.pixel_positions = self.calculate_pixel_positions()

    def calculate_pixel_positions(self):
        """
        Calculates the order of pixels based on rows and columns.
        """
        positions = []
        for row, row_dir in enumerate(self.row_directions):
            adjusted_row = (len(self.row_directions) - 1 - row) if self.ROWS_REVERSED else row
            for col in range(self.columns):
                adjusted_col = self.rows - 1 - col if row_dir else col
                for led in range(self.leds_per_pixel):
                    positions.append((adjusted_col, adjusted_row))
        return positions

    def send_image(self, image):
        """
        Sends an image to the WLED device.

        Args:
            image (object): An object with a `width` and `height` attribute
                            and a `getpixel(x, y)` method that returns the RGB value
                            for a pixel at coordinates (x, y).
        """
        assert image.width == self.columns
        assert image.height == self.rows

        data = bytearray([self.WLED_DRGB, self.WLED_TIMEOUT])
        for col, row in self.pixel_positions:
            data += bytes(image.getpixel((col, row)))
        self.udp_socket.sendto(data, self.host)
