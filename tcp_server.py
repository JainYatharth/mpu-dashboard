import socket

host = "0.0.0.0"  # Accept connections on all IP addresses
port = 1234       # Port to listen on

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((host, port))
s.listen(1)

print(f"Listening for connection on port {port}...")

conn, addr = s.accept()
print(f"Connected by {addr}")

try:
    while True:
        data = conn.recv(1024).decode()
        if not data:
            break
        print(data)
except KeyboardInterrupt:
    print("Server stopped.")
finally:
    conn.close()
    s.close()
