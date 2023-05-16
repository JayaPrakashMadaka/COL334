import sys
import os
import socket
import hashlib
import threading
import time

from socket import *

serverIP     = "192.168.58.230"
clientIP     = "192.168.58.230"	
serverUDPport= 20001

client_size = 5

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


def checkKey(dic, key):
    if key in dic.keys():
        return True
    return False
    
lock = threading.Lock()
client_dict=[]
threadsp = []

for i in range(client_size):
	client_dict.append({})
	
def perform(clientid,UDPSocket1,UDPSocket2,TCPSocket):
	while(True):
		msgFrom = UDPSocket1.recvfrom(1024)
		m = msgFrom[0].decode()
		if(m[0:4]=="Send" and checkKey(client_dict[clientid-1],int(m[12:len(m)]))):
			p=int(m[12:len(m)])
			msgClient = "Transfering"
			bytes=str.encode(msgClient)
			UDPSocket1.sendto(bytes,(serverIP,serverUDPport))
			msg=str(p)+" : "+client_dict[clientid-1][p]
			ms=str.encode(msg)
			TCPSocket.send(ms)
			msgFrom = UDPSocket2.recvfrom(1024)
			m = msgFrom[0].decode()
			if(m=="recieved"):
				continue
			else:
				print("Oops Failed")		
			

class Client:
	def __inti__(self,id):
		self.id=id
	def start(client):
		UDPClientSocket = socket(family=AF_INET, type=SOCK_DGRAM)
		UDPClientSocket.bind((clientIP,UDPPorts[client.id-1]))
		UDPCSocket = socket(family=AF_INET, type=SOCK_DGRAM)
		UDPCSocket.bind((clientIP,UPorts[client.id-1]))
		TCPServerSocket = socket(family=AF_INET, type=SOCK_STREAM)
		TCPServerSocket.bind((serverIP,TCPPorts[client.id-1]))
		TCPServerSocket.listen(1)
		msgFromClient="listening"
		bytesToSend   = str.encode(msgFromClient)
		UDPClientSocket.sendto(bytesToSend, (serverIP,serverUDPport))
		connectionSocket , addr = TCPServerSocket.accept()
		msgFrom = UDPClientSocket.recvfrom(1024)
		d=int(msgFrom[0].decode())
		msgFrom = UDPClientSocket.recvfrom(1024)
		t=int(msgFrom[0].decode())
		for i in range(t):
			message = connectionSocket.recv(2048)
			msg=message.decode()
			for i in range(len(msg)):
				if(msg[i]==':'):
					p=int(msg[0:i-1])
					client_dict[client.id-1][p]=msg[i+2:len(msg)]
					break
			UDPClientSocket.sendto(str.encode("recieved"), (serverIP,serverUDPport))
		msgFrom = UDPClientSocket.recvfrom(1024)
		m = msgFrom[0].decode()
		if(m=="Start"):
			x=threading.Thread(target=perform,args=(client.id,UDPClientSocket,UDPCSocket,connectionSocket,))
			threadsp.append(x)
			x.start()
			while(True):
				if(len(client_dict[client.id-1])==d):
					print("Got All")
					s=""
					for i in range(1,d+1):
						s+=client_dict[client.id-1][i]
					hash = hashlib.md5(s.encode()).hexdigest()
					print(hash)
					f=open("out.txt","a")
					f.write(hash+"\n")
					f.close()
					f=open("data-1 {}".format(client.id),"x")
					f.write(hash+"\n")
					f.close()
					break
				start_time=time.time()
				for i in range(1,d+1):
					if(checkKey(client_dict[c.id-1],i)==False):
						msgFromClient = "Send Packet {}".format(i)
						UDPClientSocket.sendto(str.encode(msgFromClient), (serverIP,serverUDPport))
				message = connectionSocket.recv(2048)
				msg = message.decode()
				for i in range(0,len(msg)):
					if(msg[i]==':'):
						if(checkKey(client_dict[client.id-1],int(msg[0:i-1]))==False):
							client_dict[client.id-1][int(msg[0:i-1])]=msg[i+2:len(msg)]
						break
				UDPClientSocket.sendto(str.encode("recieved"), (serverIP,serverUDPport))
				end_time=time.time()
				print(end_time-start_time)
						
						
def run(client):
	client.start()
	
threads=[]
time.sleep(1)

for i in range(client_size):
	c=Client()
	c.id=i+1
	x=threading.Thread(target=run,args=(c,))
	threads.append(x)
	x.start()
	
							
			
			
			
		
		
