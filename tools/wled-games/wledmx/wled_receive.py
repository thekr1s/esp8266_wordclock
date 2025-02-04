import socket
from PIL import Image

class WledReceive:
    """
    Receives data from WLED devices using UDP protocol and converts it into a PIL image.
    """

    WLED_DRGB = 2  # Expected protocol selection

    def __init__(self, host, port, width, height, led_per_pixel):
        self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.host = host
        self.port = port
        self.width = width
        self.height = height
        self.led_per_pixel = led_per_pixel
        self.bind_socket()

    def bind_socket(self):
        """
        Binds the UDP socket to the specified host and port.
        """
        self.udp_socket.bind((self.host, self.port))
        self.udp_socket.settimeout(1.0)

    def receive_image(self):
        """
        Receives a UDP packet and attempts to convert it into a PIL image.

        Returns:
            PIL.Image: The converted image if successful, None otherwise.
        """
        try:
            data, address = self.udp_socket.recvfrom(self.expected_packet_size())
            if data[0] != self.WLED_DRGB:
                print(f"Invalid protocol selection received. Expected: {self.WLED_DRGB}, Received: {data[0]}")
                return None
            data = data[2:]  # Skip protocol and timeout bytes

            # Reshape data into pixel order based on expected dimensions
            #print(data)
            image_data = [data[i * self.led_per_pixel * 3:(i * self.led_per_pixel + 1) * 3] for i in range(self.width * self.height)]
            image_data = b"".join(image_data)
            image = Image.frombytes("RGB", (self.width, self.height), image_data)
            return image
        except:
            return None

    def expected_packet_size(self):
        """
        Calculates the expected size of a packet based on configured settings.
        """
        return 2 + (self.width * self.height * 3) * self.led_per_pixel

# Example usage
def test():
	server = WledReceive("0.0.0.0", 21324, 5, 4, 20)
	image = server.receive_image()
	if image:
		image.show()
if __name__ == "__main__":
	test()
