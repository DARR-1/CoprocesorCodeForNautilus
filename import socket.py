import socket

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(("localhost", 27015))
client.sendall(b"test")
response = client.recv(1024)
print(response.decode())
client.close()