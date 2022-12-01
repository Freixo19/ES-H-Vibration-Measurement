import socket
import datetime;

UDP_IP = "192.168.1.209"
UDP_PORT = 2390
MESSAGE = b"Hello, World!"

print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %s" % UDP_PORT)
print("message: %s" % MESSAGE)

sock = socket.socket(socket.AF_INET,  # Internet
                     socket.SOCK_DGRAM)  # UDP

while True:
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
    data, addr = sock.recvfrom(1024)  # buffer size is 1024 bytes
    current_time = datetime.datetime.now()
    time_stamp = current_time.timestamp()
    date_time = datetime.datetime.fromtimestamp(time_stamp)
    print("Frequency: " + data.decode("utf-8") + " Timestamp: ", date_time)
