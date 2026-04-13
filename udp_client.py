import pygame as pg
import socket


def main():
    msgFromClient = "0"
    msgFromServer = None
    bytesToSend = str.encode(msgFromClient)
    # AF_INET is used for IPv4, 
    UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    serverAddressPort = ("127.0.0.1", 8000)
    buf_size = 1024

    UDP_RECV_EVENT = pg.USEREVENT
    pg.time.set_timer(UDP_RECV_EVENT, 500)

    going = True
    while going:
        for e in pg.event.get():
            if e.type == pg.KEYDOWN:
                if e.key == pg.K_ESCAPE:
                    going = False
                elif e.key == pg.K_f:
                    msgFromClient = "f"
                    bytesToSend = str.encode(msgFromClient)
                    UDPClientSocket.sendto(bytesToSend, serverAddressPort)
            if e.type == UDP_RECV_EVENT:
                UDPClientSocket.settimeout(0.150)
                try:
                    msgFromServer, _ = UDPClientSocket.recvfrom(buf_size)
                    print(f"Received: {msgFromServer}")
                except socket.timeout:
                    pass

                
if __name__ == "__main__":
    main()            
                
                    


    
