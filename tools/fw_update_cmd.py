import time
import sys
import requests


if __name__ == "__main__":
    ip = "http://192.168.2.84"
    endpoint = "/wificfg/hwcfg.html"
    url = ip + endpoint

    data = {'cl_command': 'OtaUpdate'}
    try:
        x = requests.post(url, data = data)
        print(x.text)
    finally:
        print("Done?")