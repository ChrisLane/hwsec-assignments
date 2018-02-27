#!/usr/bin/python
import serial

LN_SIZE_BIT = 256
LN_SIZE = LN_SIZE_BIT/8
E_SIZE = 2
BLOCK_SIZE = 16
BLOCK_CNT = LN_SIZE/BLOCK_SIZE
MU_SIZE = LN_SIZE + BLOCK_SIZE
CPU_FREQUENCY = 8e6

#COM_PORT = 'com22'
COM_PORT = '/dev/ttyACM1'
COM_PORT = '/dev/ttyUSB0'
BAUDRATE = 9600

GROUP_NUMBER = 0 # Group 1 has number 0, etc.

# higher timeout due to slow computation ...
ser = serial.Serial(COM_PORT, BAUDRATE, timeout=5)

# Test vectors
tv_s = [
# 0
0x82, 0x96, 0x9e, 0xc8, 0xf5, 0xea, 0xba, 0x45, 0x16, 0x68, 0x9e, 0xf6, 0xd9, 0x6b, 0x7c, 0x8d, 0x64, 0x33, 0x44, 0xcb, 0x5, 0xae, 0x45, 0x58, 0xa5, 0x3b, 0x33, 0x39, 0x1c, 0x66, 0x3d, 0xbe,
# 1
#0x4b, 0x85, 0x89, 0xaa, 0xd4, 0xea, 0xb2, 0xe5, 0x82, 0x61, 0x11, 0x8, 0xa9, 0xc1, 0x6, 0x94, 0x83, 0x63, 0x23, 0xd8, 0xf6, 0x44, 0x53, 0xf0, 0x73, 0x7c, 0xb1, 0x80, 0x1b, 0x89, 0xb5, 0xf,  
]


TV_COUNT = len(tv_s) / LN_SIZE

def unpack_le(s):
    return sum((s[i]) << (8 * i) for i in range(len(s)))
    
def pack_le(s, n):
    r = []

    for i in range(n):
        r.append(int((s >> (8 * i)) & 0xff))

    return r
    
def buf2hex(b):
    return ''.join('{:02x} '.format(ord(x)) for x in b).upper()

def array2hex(b):
    return ''.join('{:02x} '.format(x) for x in b).upper()
    
def array2buf(b):
    bb = ""
    
    for i in range(len(b)):
        bb += chr(b[i])

    return bb

for tv in range(TV_COUNT):
    print "== Test " + str(tv)
    
    s_curr = tv_s[(tv*LN_SIZE):((tv + 1)*LN_SIZE)]
  
    for rpt in range(1000):
        
        # Send operand block s
        for j in range(BLOCK_CNT):
            buf = array2buf(s_curr[j*BLOCK_SIZE:(j+1)*BLOCK_SIZE])
            
            ser.write("s")
            ser.write(buf)
            
            # read response
            rx = ser.read(2)
            #print "RX: " + buf2hex(rx)  
            
            print "s = " + buf2hex(buf)   

        # Do operation
        ser.write("e")
        ser.write(chr(GROUP_NUMBER & 0xff));
        
        # Get answer
        rx = ser.read(10)
        rx_list = map(ord, rx)
       
        # duration in CPU cycles and seconds
        duration = unpack_le(rx_list[2:])

        print "RX: " + buf2hex(rx)  
        print 
        print "=== Cycle count = " + str(duration) + " = " + str(duration/CPU_FREQUENCY) + " s ==="
        print 
        
        # Get result blocks
        c_comp = []
        for j in range(BLOCK_CNT):
            # get one block        
            ser.write("o")
            
            # read response
            rx = ser.read(BLOCK_SIZE + 2)
            rx_list = map(ord, rx)
            c_comp.extend(rx_list[:BLOCK_SIZE])
            
            print "c = "+ array2hex(rx_list[:BLOCK_SIZE])

        cCompN = unpack_le(c_comp)
        print "Got  : " + hex(cCompN)
        print       

ser.close()
