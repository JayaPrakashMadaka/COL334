import sys
import os
import socket
import hashlib
import threading
import time

from socket import *

serverIP     = "192.168.58.230"
clientIP     = "192.168.58.230"
serverUDPport = 20001
serverTCPport = 20000
bufferSize  = 2048
client_size=5


UDPports=[]
TCPS_ports=[]
TCPR_ports=[]

bu=64530
bt=20002
btr=21000

for i in range(client_size):
	UDPports.append(bu)
	TCPS_ports.append(bt)
	TCPR_ports.append(btr)
	bu+=1
	bt+=1
	btr+=1

TCPR_Sockets=[]

lock = threading.Lock()
startTime = time.time()
threads=[]
threads1=[]
threads2=[]

def checkKey(dic, key):
    if key in dic.keys():
        return True
    return False

UDPServerSocket = socket(family=AF_INET, type=SOCK_DGRAM)
UDPServerSocket.bind((serverIP , serverUDPport))

for i in range(client_size):
	TCPServerSocket = socket(family=AF_INET, type=SOCK_STREAM)
	TCPServerSocket.bind((serverIP,TCPR_ports[i]))
	TCPServerSocket.listen(1)
	TCPR_Sockets.append(TCPServerSocket)

data_dict = {}
cache = {}
queries={}
cache_size = 5

d=1
f = open("/home/jp/Downloads/A2_small_file.txt", 'r')
while True:
    piece = f.read(800)  
    if not piece:
        break
    data_dict[d]=piece
    d+=1
f.close()

d-=1

direc=[ d//client_size for i in range(client_size)]
	
x=d%client_size
ptr=0
while(x !=0):
	direc[ptr]+=1
	ptr=(ptr+1)%client_size
	x-=1


for i in range(1,d+1):
	queries[i]=[]

def sendUDP(i,message):
	global lock
	with lock:
		clientaddressPort=(clientIP,UDPports[i])
		msg=str.encode(message)
		UDPServerSocket.sendto(msg,clientaddressPort)	


def transfer(i):
	while(True):
		connectSocket , addr = TCPR_Sockets[i].accept()
		msg=connectSocket.recv(1024)
		connectSocket.close()
		message=msg.decode()
		if(message[0:4]=="Send"):
			p=int(message[12:len(message)])
			if(checkKey(cache,p)):
				m=cache[p]
				msgfromServer=str(p)+" : "+m
				x=threading.Thread(target=sendUDP,args=(i,msgfromServer,))
				threads.append(x)
				x.start()
			else:
				queries[p].append(i)
				for j in range(client_size):
					msgfromServer="Send Packet {}".format(p)
					TCPClientSocket = socket(AF_INET, SOCK_STREAM)
					TCPClientSocket.connect((clientIP,TCPS_ports[j]))
					msg=str.encode(msgfromServer)
					TCPClientSocket.send(msg)
					TCPClientSocket.close()



TCPServerSocket = socket(family=AF_INET, type=SOCK_STREAM)
TCPServerSocket.bind((serverIP,serverTCPport))
TCPServerSocket.listen(10)

print("Server up and listening")

count=0
while(True and count < client_size):
	connectionSocket, addr = TCPServerSocket.accept()
	msg=connectionSocket.recv(100)
	message=msg.decode()
	if(message == "listening"):
		count+=1
	connectionSocket.close()

for i in range(client_size):
	msg=str(d)+","+str(direc[i])
	TCPClientSocket = socket(AF_INET, SOCK_STREAM)
	TCPClientSocket.connect((clientIP,TCPS_ports[i]))
	TCPClientSocket.send(str.encode(msg))
	TCPClientSocket.close()


for i in range(1,d+1):
	msgfromServer=str(i)+" : "+data_dict[i]
	x=threading.Thread(target=sendUDP,args=((i-1)%client_size,msgfromServer,))
	threads.append(x)
	x.start()
	data_dict.pop(i)
	
for i in range(client_size):
	y=threading.Thread(target=transfer,args=(i,))
	threads1.append(y)
	y.start()

while(True):
	bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)
	msg = bytesAddressPair[0].decode()
	for i in range(len(msg)):
		if(msg[i]==":"):
			p=int(msg[0:i-1])
			print("Recieved {}".format(p))
			cache[int(msg[0:i-1])]=msg[i+2:len(msg)]
			for a in queries[p]:
				x=threading.Thread(target=sendUDP,args=(a,msg,))
				threads.append(x)
				x.start()
			break	
			
	

endTime = time.time()	
