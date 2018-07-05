# python3
import socket
import sys
import binascii
import struct

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
ip = "127.0.0.1"
port = 7123

sock.bind((ip, port))

try:
    # Receive response
    print("listening on port {0}...".format(port))

    while True:
        data, server = sock.recvfrom(4096)

        sys.stdout.write("received:")
        hexdata = binascii.hexlify(data)
        print(hexdata, len(data))

        i = 0
        sz = 8
        print(i)
        ptr64_log = int.from_bytes(data[i:i+sz], byteorder=sys.byteorder, signed=False)         #8
        i += sz

        sz = 4
        print(i)
        sint32_status = int.from_bytes(data[i:i+sz], byteorder=sys.byteorder, signed=True)    #2
        i += sz

        # Global
        sz = 8
        print(i)
        uint64_hdr_bufsz = int.from_bytes(data[i:i+sz], byteorder=sys.byteorder, signed=False)  #8
        # sint32_hdr_nbufs = data[14:]  #4
        # uint64_hdr_bytes = data[16:]  #8

        print(ptr64_log, sint32_status, uint64_hdr_bufsz)

        # uint64_data_bufsz = data[24:]  #8
        # sint32_data_nbufs = data[32:]  #4
        # uint64_data_bytes = data[34:]  #8
        # sint32_n_readers = data[42:]   #4
        #
        # # per reader stats
        # uint64_hdr_bufs_written = data[44:]    #8
        # uint64_hdr_bufs_read = data[52:]       #8
        # uint64_hdr_full_bufs = data[60:]       #8
        # uint64_hdr_clear_bufs = data[68:]      #8
        # uint64_hdr_available_bufs = data[76:]  #8
        #
        # uint64_data_bufs_written = data[84:]            #8
        # uint64_data_bufs_read = data[92:]               #8
        # uint64_data_full_bufs = data[100:]               #8
        # uint64_data_clear_bufs = data[108:]              #8
        # uint64_data_available_bufs = data[116:151]       #8

        sys.stdout.write("\n")
        sys.stdout.flush()
finally:
    print("closing socket")
    sock.close()
