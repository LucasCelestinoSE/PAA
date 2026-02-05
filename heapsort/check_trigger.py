
with open('roteador.input', 'r') as f:
    header = f.readline().split()
    total_packets = int(header[0])
    capacity = int(header[1])
    
    current_bytes = 0
    buffer = []
    
    for i in range(total_packets):
        line = f.readline().split()
        if not line: break
        priority = int(line[0])
        size = int(line[1])
        
        if current_bytes + size > capacity:
            # Trigger!
            buffer.sort(key=lambda x: x[0], reverse=True) # Max priority first
            print(f"Trigger at packet {i}, total bytes {current_bytes}")
            print(f"First packet in sorted buffer: Priority {buffer[0][0]}")
            break
        
        current_bytes += size
        buffer.append((priority, size))
