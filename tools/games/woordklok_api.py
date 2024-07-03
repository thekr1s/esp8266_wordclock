import socket
import requests

class Woordklok:
    """
    REST interface of the woordklok
    """
    WLED_DRGB = 2     # Protocol selection
    WLED_TIMEOUT = 5  # Timeout in seconds

    COLUMNS = 13
    ROWS = 13
    def __init__(self, host):
        self.host = host
        self.urlStart = "http://" + host[0]
        self.udp_socket = None
        
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
            self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def send_image(self, image):
        """
        Sends an image to the WLED device.

        Args:
            image (object): An object with a `width` and `height` attribute
                            and a `getpixel(x, y)` method that returns the RGB value
                            for a pixel at coordinates (x, y).
        """
        assert image.width == self.COLUMNS
        assert image.height == self.ROWS

        if self.udp_socket == None:
            print("Realtime UDP not started, starting....")
            self.start_realtime_udp()

        data = bytearray([self.WLED_DRGB, self.WLED_TIMEOUT])
        
        for row in range(self.ROWS):
            for col in range(self.COLUMNS):
                data += bytes(image.getpixel((col, row)))
        self.udp_socket.sendto(data, self.host)


if __name__ == '__main__':
    api = Woordklok(("192.168.2.87", "80"))
    api.start_update()
    
    # api.start_realtime_udp()