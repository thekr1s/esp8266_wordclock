import socket
import requests

class Woordklok:
    """
    REST interface of the woordklok
    """
    def __init__(self, host):
        self.host = host
        self.urlStart = "http://" + host[0]
        
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


if __name__ == '__main__':
    api = Woordklok(("192.168.2.87", "80"))
    api.start_update()
    
    # api.start_realtime_udp()