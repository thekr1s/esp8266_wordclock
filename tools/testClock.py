import time
import socket
import sys


if __name__ == "__main__":
    ip = "schoonheidssalonsuzanne.nl"
    port = 80
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = (ip, port)
    sock.connect(server_address)
    # Send POST request.
    postRequest = "GET /here_am_i/get_with_age.php HTTP/1.1\r\n"
    postRequest += "Host: schoonheidssalonsuzanne.nl\r\n"
    postRequest += "Connection: keep-alive\r\nAccept: */*\r\n\r\n"
    print postRequest
    try:
        sock.sendall(postRequest)
        print sock.recv(4096)
    finally:
        time.sleep(1)
        sock.close()
        time.sleep(1)
    