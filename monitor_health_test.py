# python3
import socket
import sys
import struct
import datetime

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

        fmt = "=iQiQQiQiQQQQQQQQQQ"
        (sint32_status, uint64_hdr_bufsz, sint32_hdr_nbufs, uint64_hdr_bytes, uint64_data_bufsz, sint32_data_nbufs,
         uint64_data_bytes, sint32_n_readers, uint64_hdr_bufs_written, uint64_hdr_bufs_read, uint64_hdr_full_bufs,
         uint64_hdr_clear_bufs, uint64_hdr_available_bufs, uint64_data_bufs_written, uint64_data_bufs_read,
         uint64_data_full_bufs, uint64_data_clear_bufs, uint64_data_available_bufs) = struct.unpack(fmt, data)

        print("{0}: Status: {1} Readers: {2} Buffs tot: {3} {4}MB avail: {5} {6}MB clear: {7} full: {8} read: {9} {10} MB".format(
            datetime.datetime.now().strftime("%Y-%m-%d %H:%M"), sint32_status, sint32_n_readers,
            sint32_data_nbufs, (uint64_data_bytes / (1000*1000)),
            uint64_data_available_bufs, (uint64_data_available_bufs*uint64_data_bufsz/(1000*1000)),
            uint64_data_clear_bufs,
            uint64_data_full_bufs,
            uint64_data_bufs_read, (uint64_data_bufs_read*uint64_data_bufsz/(1000*1000))))
        sys.stdout.flush()
finally:
    print("closing socket")
    sock.close()
