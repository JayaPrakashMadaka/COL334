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
serverUDPSport = 20000

def checkKey(dic, key):
    if key in dic.keys():
        return True
    return False

client_size = 5
cache_size = 5

sent=[0 for i in range(client_size)]
clients_lock=[1 for i in range(client_size)]

UDPPorts=[]
UPorts=[]
TCPPorts=[]

bt=21002
bu=64530
b=63530

for i in range(client_size):
	TCPPorts.append(bt)
	UDPPorts.append(bu)
	UPorts.append(b)
	b+=1
	bt+=1
	bu+=1
	


TCPSockets=[]

map={}
for i in range(client_size):
	map[UDPPorts[i]]=i

data_dict={}

cache={}

threads=[]
threadsu=[]
threadsr = []
lock = threading.Lock()


d=1
f = open("/home/jp/Downloads/A2_small_file.txt", 'r')
while True:
    piece = f.read(800)  
    if not piece:
        break
    data_dict[d]=piece
    if(d<cache_size):
    	cache[d]=piece
    d+=1
f.close()

d-=1

print(d)

direc=[ d//client_size for i in range(client_size)]
	
x=d%client_size
ptr=0
while(x !=0):
	direc[ptr]+=1
	ptr=(ptr+1)%client_size
	x-=1


UDPServerSocket = socket(family=AF_INET, type=SOCK_DGRAM)
UDPServerSocket.bind((serverIP , serverUDPport))

def sendUDP(message,client):
	msg=str.encode(message)
	UDPServerSocket.sendto(msg,(clientIP,UDPPorts[client]))


def transmit(packet,client):
	if(checkKey(cache,packet)==False):
		msg="Send Packet {}".format(packet)
		for i in range(client_size):
			x=threading.Thread(target=sendUDP,args=(msg,i,))
			threadsu.append(x)
			x.start()
	while(checkKey(cache,packet)==False or clients_lock[client]==0):
		continue
	msg=cache[packet]
	clients_lock[client]=0
	msgfromServer=str(packet)+" : "+msg
	TCPSockets[client].send(str.encode(msgfromServer))
	sent[client]+=1
	print("{} number {}".format(client+1,sent[client]))
		
	
def recieve(port):
	msg=TCPSockets[map[port]].recv(2048)
	p=0
	data=""
	msg=msg.decode()
	for i in range(len(msg)):
		if(msg[i]==":"):
			p=int(msg[0:i-1])
			data=msg[i+2:len(msg)]
			break
	if(checkKey(cache,p)==False):
		if(len(cache)<cache_size):
			cache[p]=data
		else:
			cache.popitem()
			cache[p]=data
	UDPServerSocket.sendto(str.encode("recieved"),(clientIP,UPorts[map[port]]))
				
print("Server Up and Listening")

for i in range(client_size):
	bytesAddressPair = UDPServerSocket.recvfrom(1024)
	message = bytesAddressPair[0].decode()
	IP , port = bytesAddressPair[1]
	if(message=="listening"):
		TCPClientSocket = socket(AF_INET, SOCK_STREAM)
		TCPClientSocket.connect((clientIP,TCPPorts[i]))
		TCPSockets.append(TCPClientSocket)
		UDPServerSocket.sendto(str.encode(str(d)),(clientIP,port))
		UDPServerSocket.sendto(str.encode(str(direc[i])),(clientIP,port))
for i in range(1,d+1):
	msgfromServer=str(i)+" : "+data_dict[i]
	TCPSockets[(i-1)%client_size].send(str.encode(msgfromServer))
	bytesAddressPair = UDPServerSocket.recvfrom(1024)
	message = bytesAddressPair[0].decode()
	if(message == "recieved"):
		continue
	else:
		print("Oops Failed")
	data_dict.pop(i)
for i in range(client_size):
	msg="Start"
	UDPServerSocket.sendto(str.encode(msg),(clientIP,UDPPorts[i]))
	

while(True):
	bytesAddressPair = UDPServerSocket.recvfrom(1024)
	message = bytesAddressPair[0].decode()
	IP , port = bytesAddressPair[1]
	if(message[0:4]=="Send"):
		x=threading.Thread(target=transmit,args=(int(message[12:len(message)]),map[port],))
		threads.append(x)
		x.start()
	if(message=="Transfering"):
		x=threading.Thread(target=recieve,args=(port,))
		threadsr.append(x)
		x.start()
	if(message=="recieved"):
		clients_lock[map[port]]=1
		
		

