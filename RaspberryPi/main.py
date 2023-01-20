import socket
import datetime
from scipy.signal import windows
from scipy.fft import rfft, rfftfreq
import time
import numpy as np
import influxdb_client, os, time
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS
import threading

token = "IWB-sAsi08hSt9-MoCrZi_jHxyQZPaMfCBUCIPePlcr6BofvP8NXX83ouhGfVT6AjbAEG86aRSEWTeev17r1iQ=="

org = "esequipahinfluxseila@gmail.com"
url = "https://eu-central-1-1.aws.cloud2.influxdata.com"

client = influxdb_client.InfluxDBClient(url=url, token=token, org=org)
bucket = "Vibration Measurements"

write_api = client.write_api(write_options=SYNCHRONOUS)

UDP_PORT = 13325
MESSAGE = b"Hello Arduino!"

result_x = 0
result_y = 0
result_z = 0


def get_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(0)
    try:
        # doesn't even have to be reachable
        s.connect(('10.254.254.254', 1))
        IP = s.getsockname()[0]
    except Exception:
        IP = '127.0.0.1'
    finally:
        s.close()
    return IP


def fft_function(array, l):
    global result_x
    global result_y
    global result_z
    data_to_process = np.array(array)
    
    fft_result = rfft(data_to_process)
    
    frequencies = rfftfreq(len(data_to_process), 0.002)
    
    index = fft_result.argmax()
    
    frequency = abs(frequencies[index])
    
    if frequency == 0:
        
        index = np.argsort(np.abs(fft_result))[::-1][1:2]
        
        frequency = abs(frequencies[index])
        
    if abs(fft_result[index]) <= 2:
        index = fft_result.argmax()
        frequency = 0
        
    elif frequency > 100:
        index_counter = 2
        while frequency >100:
            
            index = np.argsort(np.abs(fft_result))[::-1][(index_counter-1):index_counter]
            frequency = abs(frequencies[index])
            index_counter = index_counter + 1
            
            if abs(fft_result[index]) <= 2:
                index = fft_result.argmax()
                frequency = 0
            
    if l == 0:
        result_x = float(frequency)
        
    elif l == 1:
        result_y = float(frequency)
        
    else:
        result_z = float(frequency)
        


string_ip = get_ip().split(".")


sock = socket.socket(socket.AF_INET,  # Internet
                     socket.SOCK_DGRAM)  # UDP
sock.settimeout(0.2)
ip_counter = 1
while 1 :

    if str(ip_counter) == string_ip[3]:
        
        ip_counter = ip_counter + 1
        continue
    else:
        UDP_IP = string_ip[0] + "." + string_ip[1] + "." + string_ip[2] + "." + str(ip_counter)
        
        

        sock.sendto(MESSAGE, (str(UDP_IP), UDP_PORT))
        
        try:
            
            data, addr = sock.recvfrom(1024)
            if (data):
                UDP_IP = addr
                
                break
        except socket.timeout:
            
            ip_counter = ip_counter + 1
            if ip_counter >= 255:
                ip_counter = 1
                
            pass

MESSAGE = b"Send Data"
counter = 0
sock.settimeout(5)
start = time.time()

master_array_x = []
master_array_y = []
master_array_z = []
master_counter = 0
while True:
    try:
        sock.sendto(MESSAGE, UDP_IP)

        data, addr = sock.recvfrom(1500)  # buffer size is 1500 bytes
        current_time = datetime.datetime.now()
        time_stamp = current_time.timestamp()
        date_time = datetime.datetime.fromtimestamp(time_stamp)
        
        checksum = 0
        aux = bytearray()
        char = []
        freq = bytearray()

        array_x = []
        array_y = []
        array_z = []

        counter = 0
        
        for i in range(len(data) - 1):
            if data[i] == 78 or data[i] == 80:
                freq = float(aux.decode("utf-8"))
                if data[i] == 78:
                    freq = freq * (-1)
                char.append(freq)
                counter = counter + 1
                
                if counter >= 3:
                    array_x.append(char[0])
                    array_y.append(char[1])
                    array_z.append(char[2])
                    del char
                    char = []
                    counter = 0
                aux = bytearray()
            else:
                aux.append(data[i])

            checksum ^= data[i]
        if checksum == data[len(data) - 1]:
            for k in range(len(array_x)):
                master_array_x.append(array_x[k])
                master_array_y.append(array_y[k])
                master_array_z.append(array_z[k])
            master_counter = master_counter + 64

        if master_counter >= 256:
            thread_x = threading.Thread(target=fft_function, args=(master_array_x, 0,))
            thread_y = threading.Thread(target=fft_function, args=(master_array_y, 1,))
            thread_z = threading.Thread(target=fft_function, args=(master_array_z, 2,))
            thread_x.start()
            thread_y.start()
            thread_z.start()

            thread_x.join()
            thread_y.join()
            thread_z.join()

            master_counter = 0
            del master_array_z
            del master_array_x
            del master_array_y
            master_array_x = []
            master_array_y = []
            master_array_z = []

            pointX = (Point("Arduino_1")
                    .tag("Frequency_tag", "FrequencyValue")
                    .field("FrequencyX", result_x))
            
            pointY = (Point("Arduino_1")
                    .tag("Frequency_tag", "FrequencyValue")
                    .field("FrequencyY", result_y))
            
            pointZ = (Point("Arduino_1")
                    .tag("Frequency_tag", "FrequencyValue")
                    .field("FrequencyZ", result_z))
            
            write_api.write(bucket=bucket, org="esequipahinfluxseila@gmail.com", record=[pointX, pointY, pointZ])

    except socket.timeout:
        print('Timeout')
        pass
