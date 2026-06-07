from itertools import cycle

# 1. Define your plaintext and key as wide-character strings
plaintext = "CPE1704TKS"
key = "tribe"

# 2. Encode strings into bytes using an encoding that matches wchar_t
#    'utf-32-le' = 4-byte characters, little-endian.
#    Use 'utf-16-le' for 2-byte wchar_t (like on Windows).
data_bytes = plaintext.encode('utf-32-le')
key_bytes = key.encode('utf-32-le')

# 3. Perform the byte-wise XOR operation
encrypted_bytes = bytes([d ^ k for d, k in zip(data_bytes, cycle(key_bytes))])

# 4. Print the result as a formatted C-style byte array
print("static const uint8_t data[] = {")
for i, byte in enumerate(encrypted_bytes):
    if i % 12 == 0:
        print("    ", end="")
    print(f"0x{byte:02X}, ", end="")
    if i % 12 == 11:
        print()
print("\n};")
print(f"// Total size: {len(encrypted_bytes)} bytes")