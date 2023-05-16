import sys
import os
import socket
import hashlib
import threading
import time

from socket import *

serverIP     = "192.168.58.230"
clientIP     = "192.168.58.230"
bufferSize  = 2048
serverUDP = 20001
serverTCPport=20000

number_clients=5

UDPports=[]
TCPS_ports=[]
TCPR_ports=[]

bu=64530
bt=20002
btr=21000

for i in range(number_clients):
	UDPports.append(bu)
	TCPS_ports.append(bt)
	TCPR_ports.append(btr)
	bu+=1
	bt+=1
	btr+=1

def checkKey(dic, key):
    if key in dic.keys():
        return True
    return False

lock = threading.Lock()
startTime = time.time()
threads=[]
threads1=[]

client_dict=[]

for i in range(number_clients):
	client_dict.append({})



def recv(id,TCPServerSocket,UDPClientSocket):
	while(True):
		connectionSocket,addr=TCPServerSocket.accept()
		message=connectionSocket.recv(1024)
		msg=message.decode()
		if(msg[0:4]=="Send"):
			p=int(msg[12:len(msg)])
			if(checkKey(client_dict[id-1],p)):
				msgfromclient=str.encode(str(p)+" : "+client_dict[id-1][p])
				UDPClientSocket.sendto(msgfromclient,(serverIP,serverUDP))
		connectionSocket.close()
		

class Client:
	def __inti__(self,id):
		self.id=id
	def start(client):
		clientUDP=UDPports[client.id-1]
		clientTCP=TCPS_ports[client.id-1]
		serverTCP=TCPR_ports[client.id-1]
		UDPClientSocket = socket(family=AF_INET, type=SOCK_DGRAM)
		UDPClientSocket.bind((clientIP,clientUDP))
		TCPClientSocket = socket(AF_INET, SOCK_STREAM)
		TCPClientSocket.connect((serverIP,serverTCPport))
		msg="listening"
		TCPClientSocket.send(str.encode(msg))
		TCPClientSocket.close()
		TCPServerSocket = socket(family=AF_INET, type=SOCK_STREAM)
		TCPServerSocket.bind((clientIP,clientTCP))
		TCPServerSocket.listen(1)
		
		print("Connected")
		c=0
		c1=0
		d=0
		connectionSocket , addr = TCPServerSocket.accept()
		msg=connectionSocket.recv(1024)
		connectionSocket.close()
		msg=msg.decode()
		for i in range(len(msg)):
			if(msg[i]==","):
				c1=int(msg[i+1:len(msg)])
				d = int(msg[0:i])
				break
		while(True and c<c1):
			c+=1
			bytesAddressPair = UDPClientSocket.recvfrom(bufferSize)
			msg = bytesAddressPair[0].decode()
			address = bytesAddressPair[1]
			for i in range(len(msg)):
				if(msg[i]==":"):
					client_dict[client.id-1][int(msg[0:i-1])]=msg[i+2:len(msg)]
					break

			x=threading.Thread(target=recv,args=(client.id,TCPServerSocket,UDPClientSocket,))
			threads.append(x)
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
				f=open("data {}".format(client.id),"x")
				f.write(s)
				f.close()
				break
			for i in range(1,d+1):
				if(checkKey(client_dict[client.id-1],i)==False):
					TCPClientSocket = socket(AF_INET, SOCK_STREAM)
					TCPClientSocket.connect((serverIP,serverTCP))
					TCPClientSocket.send(str.encode("Send Packet {}".format(i)))
					TCPClientSocket.close()
					break	
			bytesAddressPair = UDPClientSocket.recvfrom(bufferSize)
			msg = bytesAddressPair[0].decode()
			address = bytesAddressPair[1]
			for i in range(len(msg)):
				if(msg[i]==":"):
					client_dict[client.id-1][int(msg[0:i-1])]=msg[i+2:len(msg)]
					break

threadss=[]

def run(client):
	client.start()

time.sleep(1)			
for i in range(number_clients):
	c=Client()
	c.id = i-1
	x=threading.Thread(target=run,args=(c,))
	threadss.append(x)
	x.start()


endTime = time.time()	
