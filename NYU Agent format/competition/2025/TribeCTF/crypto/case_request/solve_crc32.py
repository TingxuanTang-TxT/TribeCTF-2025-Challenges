#!/usr/bin/env python3

# from pwn import remote

# crc32 polynomial with x^31 coeff as least bit
POLY = 0xEDB88320

# srv = remote("todo", "change me")


def crc32(data: bytes) -> int:
    rem = 0xFFFFFFFF
    for b in data:
        rem ^= b
        for _ in range(8):
            if rem & 1:
                rem = (rem >> 1) ^ POLY
            else:
                rem >>= 1
    return rem ^ 0xFFFFFFFF


def xor(a: bytes, b: bytes) -> bytes:
    #return bytes([byte_a ^ byte_b for byte_a, byte_b in zip(a, b, strict=True)])
    return bytes([byte_a ^ byte_b for byte_a, byte_b in zip(a, b)])


def p32(val):
    print(f"{val:032b}")


def forge_crc(old, new):
    difference = xor(old, new)  # same as old ^ new ^ zeros
    return crc32(difference) ^ crc32(b"\x00" * len(old))


token = bytes.fromhex(input("token: "))
crc = token[12:16]
payload = token[16:]


"""
new = old ^ diff ^ zeros
crc32(new) = crc32(old ^ diff ^ zeros) = crc32(old) ^ (crc32(diff) ^ crc32(zeros))

cipher ^ (msg + crc)


{"admin": false, "username":"...", "user_id": "???????"}
{"admin": true,  "username":"...", "user_id": "???????"}
"""
second = b'{"approved":false,'
first_ = b'{"approved":true, '
diff = xor(first_, second) + b"\x00" * (len(payload) - len(first_))
crc_diff = (crc32(diff) ^ crc32(b"\x00" * len(payload))).to_bytes(4, "little")
new_token = token[:12] + xor(crc + payload, crc_diff + diff)
print("token:", new_token.hex())
