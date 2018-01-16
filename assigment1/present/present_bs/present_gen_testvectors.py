#!/usr/bin/python
import serial
from pypresent import Present

BLOCK_SIZE = 8
KEY_SIZE = 10

# 3 pt
tv_pt = [
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,           
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,    
    0x45, 0x84, 0x22, 0x7B, 0x38, 0xC1, 0x79, 0x55,
]

# 3 keys
tv_key = [
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x3c, 0xf4, 0x00, 0xd8, 0x28, 0xf1, 0x08, 0x7a, 0x60, 0x26,
]

TV_COUNT = len(tv_pt) / BLOCK_SIZE

def unpack_le(s):
    return sum((s[i]) << (8 * i) for i in range(len(s)))
    
def buf2hex(b):
    return ''.join('{:02x} '.format(ord(x)) for x in b).upper()
    
def buf2hex_rev(b):
    return ''.join('0x{:02x}, '.format(ord(x)) for x in reversed(b)).lower()

def array2buf(b):
    bb = ""
    
    for i in range(len(b)):
        bb += chr(b[i])

    return bb
    
def array2buf_rev(b):
    bb = ""
    
    for i in reversed(range(len(b))):
        bb += chr(b[i])

    return bb

for i_key in range(TV_COUNT):
    for i_pt in range(TV_COUNT):    
        print "#  Pt " + str(i_pt) + ", key " + str(i_key)
        
        key = array2buf_rev(tv_key[i_key*KEY_SIZE:(i_key+1)*KEY_SIZE])
        buf = array2buf_rev(tv_pt[i_pt*BLOCK_SIZE:(i_pt+1)*BLOCK_SIZE])
        
        c = Present(key);
        print buf2hex_rev(c.encrypt(buf))
