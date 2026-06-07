import subprocess
from pathlib import Path
from Crypto.Signature import pkcs1_15
from Crypto.Hash import SHA256
from Crypto.PublicKey import RSA
from Crypto.Util.number import bytes_to_long, long_to_bytes
from tinyaes import AES
from Crypto.Cipher import AES as pyaes
import os
import struct
from Crypto.Util.Padding import pad
import ctypes
from ctypes import c_int, c_size_t, c_void_p, c_uint8, POINTER, create_string_buffer, string_at

FLASH_SIZE = 1024 * 1024  # 1MB

# --- Build Functions ---

def build_stage1_flash():
    build_dir = Path("../flash_rom_stage1/")
    try:
        subprocess.run(["make"], cwd=build_dir, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Build failed: {e}")
        exit(1)

def build_stage2_flash():
    build_dir = Path("../flash_rom_stage2/")
    try:
        subprocess.run(["make"], cwd=build_dir, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Build failed: {e}")
        exit(1)

# --- Cryptography and File Prep Functions ---

def generate_sig_blocks(message: bytes) -> tuple:
    with open('keys/private_key.pem', 'r') as f:
        private_key = RSA.import_key(f.read())

    public_key = private_key.public_key()
    h = SHA256.new(message)
    signer = pkcs1_15.new(private_key)
    signature = signer.sign(h)

    print("Generated signature block.")

    verifier = pkcs1_15.new(public_key)
    try:
        verifier.verify(h, signature)
        print("The signature is valid.")
    except (ValueError, TypeError):
        print("The signature is not valid.")

    modulus = public_key.n
    exponent = public_key.e

    return signature, (modulus, exponent)

class LZOCompressor:
    """A class to safely manage the LZO library and its resources."""
    def __init__(self):
        try:
            self.lzo_lib = ctypes.CDLL('./crypto_libs/liblzo.so')
        except OSError as e:
            print(f"Error loading liblzo.so: {e}")
            exit(1)

        # Set up function prototypes
        self.lzo_lib.py_lzo_init.restype = c_int
        self.lzo_lib.py_lzo_compress.argtypes = [
            POINTER(c_uint8), c_size_t,
            POINTER(c_uint8), POINTER(c_size_t),
            c_void_p
        ]
        self.lzo_lib.py_lzo_compress.restype = c_int

        if self.lzo_lib.py_lzo_init() != 0:
            raise RuntimeError("LZO initialization failed.")

        # Keep work memory as an instance attribute
        self.lzo_workmem = create_string_buffer(16384) # LZO1X_1_MEM_COMPRESS

    def compress(self, data: bytes) -> bytes:
        """Compresses data using the loaded LZO library."""
        in_len = len(data)
        out_max = in_len + in_len // 16 + 64 + 3

        in_buf = create_string_buffer(data, in_len)
        out_buf = create_string_buffer(out_max)
        out_len = c_size_t(out_max)

        res = self.lzo_lib.py_lzo_compress(
            ctypes.cast(in_buf, POINTER(c_uint8)), in_len,
            ctypes.cast(out_buf, POINTER(c_uint8)), ctypes.byref(out_len),
            ctypes.cast(self.lzo_workmem, c_void_p)
        )

        if res != 0:
            raise RuntimeError(f"LZO compression failed with error code {res}")
        
        print(f"Compressed size: {out_len.value}")
        return string_at(out_buf, out_len.value)

def encrypt_flash_img(compressed_data: bytes) -> tuple[bytes, bytes, bytes]:
    """
    Encrypts the data using tinyaes (CBC mode) so it matches tiny-AES-c.
    Returns the encrypted data, key, and IV.
    """
    key = b",\t\x0eM\x8d\xc9\x17\xb3\x0ewy \x10\x10\xe6\x89"
    iv = b'\xb5;\x07_1\x99%S\xb8n\x10\xd1!\x96\x9a\x83'

    # Pad the data to a multiple of 16 bytes (AES block size)
    padded = pad(compressed_data, 16)

    # Encrypt with tinyaes (AES-CBC)
    aes = AES(key, iv)
    aes.CBC_encrypt_buffer_inplace_raw(padded)

    print(f"Encryption successful. Encrypted size: {len(padded)}")
    return padded, key, iv

def prepare_flash_img_user(data1: bytes, sig1: bytes, encrypted: bytes, sig2: bytes):
    """
    Writes all the component parts to the final flash.img file.
    """
    print("\n--- Flash Image Memory Map ---")
    offset = 0

    # --- Stage 1 ---
    size = len(data1)
    start_addr = offset
    end_addr = offset + size - 1
    print(f"{'Stage 1':<12} | Size: {size:<6} (0x{size:X}) | Addr: 0x{start_addr:08X} - 0x{end_addr:08X}")
    offset += size

    # --- Signature 1 ---
    size = len(sig1)
    start_addr = offset
    end_addr = offset + size - 1
    print(f"{'Signature 1':<12} | Size: {size:<6} (0x{size:X}) | Addr: 0x{start_addr:08X} - 0x{end_addr:08X}")
    offset += size

    # --- Encrypted Stage 2 ---
    size = len(encrypted)
    start_addr = offset
    end_addr = offset + size - 1
    print(f"{'Encrypted S2':<12} | Size: {size:<6} (0x{size:X}) | Addr: 0x{start_addr:08X} - 0x{end_addr:08X}")
    offset += size

    # --- Signature 2 ---
    size = len(sig2)
    start_addr = offset
    end_addr = offset + size - 1
    print(f"{'Signature 2':<12} | Size: {size:<6} (0x{size:X}) | Addr: 0x{start_addr:08X} - 0x{end_addr:08X}")
    offset += size

    # --- Fill Data ---
    fill_size = FLASH_SIZE - offset
    start_addr = offset
    end_addr = offset + fill_size - 1
    print(f"{'Fill Data':<12} | Size: {fill_size:<6} (0x{fill_size:X}) | Addr: 0x{start_addr:08X} - 0x{end_addr:08X}")

    print("------------------------------------------------------------------")
    print(f"{'Total Image':<12} | Size: {FLASH_SIZE:<6} (0x{FLASH_SIZE:X}) | Addr: 0x00000000 - 0x{FLASH_SIZE - 1:08X}")
    
    fill_size = FLASH_SIZE - (len(data1) + len(sig1) + len(encrypted) + len(sig2))
    with open('/dev/urandom', 'rb') as fp:
        fill_data = fp.read(fill_size)
    
    with open('flash.img', 'wb') as fp:
        fp.write(data1)
        fp.write(sig1)
        fp.write(encrypted)
        fp.write(sig2)
        fp.write(fill_data)
        
    print("Final flash.img created successfully.")

def prepare_flash_img_flag(data1: bytes, sig1: bytes, encrypted: bytes, sig2: bytes, flag: bytes):
    """
    Prepares hidden flash image with flash embedded and prints the memory map.
    """
    print("\n--- Flash Image Memory Map (w/ Flag) ---")
    offset = 0

    # --- Stage 1 ---
    size = len(data1)
    start_addr = offset
    end_addr = offset + size - 1
    print(f"{'Stage 1':<12} | Size: {size:<6} (0x{size:X}) | Addr: 0x{start_addr:08X} - 0x{end_addr:08X}")
    offset += size

    # --- Signature 1 ---
    size = len(sig1)
    start_addr = offset
    end_addr = offset + size - 1
    print(f"{'Signature 1':<12} | Size: {size:<6} (0x{size:X}) | Addr: 0x{start_addr:08X} - 0x{end_addr:08X}")
    offset += size

    # --- Encrypted Stage 2 ---
    size = len(encrypted)
    start_addr = offset
    end_addr = offset + size - 1
    print(f"{'Encrypted S2':<12} | Size: {size:<6} (0x{size:X}) | Addr: 0x{start_addr:08X} - 0x{end_addr:08X}")
    offset += size

    # --- Signature 2 ---
    size = len(sig2)
    start_addr = offset
    end_addr = offset + size - 1
    print(f"{'Signature 2':<12} | Size: {size:<6} (0x{size:X}) | Addr: 0x{start_addr:08X} - 0x{end_addr:08X}")
    offset += size
    
    # --- Flag ---
    size = len(flag)
    start_addr = offset
    end_addr = offset + size - 1
    print(f"{'Flag':<12} | Size: {size:<6} (0x{size:X}) | Addr: 0x{start_addr:08X} - 0x{end_addr:08X}")
    offset += size

    # --- Fill Data ---
    fill_size = FLASH_SIZE - offset
    start_addr = offset
    end_addr = offset + fill_size - 1
    print(f"{'Fill Data':<12} | Size: {fill_size:<6} (0x{fill_size:X}) | Addr: 0x{start_addr:08X} - 0x{end_addr:08X}")

    print("------------------------------------------------------------------")
    print(f"{'Total Image':<12} | Size: {FLASH_SIZE:<6} (0x{FLASH_SIZE:X}) | Addr: 0x00000000 - 0x{FLASH_SIZE - 1:08X}")
    
    # --- Write the final image file ---
    with open('/dev/urandom', 'rb') as fp:
        fill_data = fp.read(fill_size)
    
    with open('flash_server.img', 'wb') as fp:
        fp.write(data1)
        fp.write(sig1)
        fp.write(encrypted)
        fp.write(sig2)
        fp.write(flag)
        fp.write(fill_data)
        
    print("\nFinal flash_server.img created successfully.")

def generate_reversible_block(message: bytes) -> tuple[bytes, tuple[int,int]]:
    """
    Performs a raw RSA private key operation on the message.
    WARNING: This is NOT a secure signature and is for educational/CTF purposes ONLY.
    """
    with open('keys/private_key.pem', 'r') as f:
        private_key = RSA.import_key(f.read())

    # --- This is the core of the insecure, reversible operation ---

    # 1. Convert the message bytes to an integer
    message_as_int = bytes_to_long(message)

    # Sanity check: In raw RSA, the message must be smaller than the modulus.
    if message_as_int >= private_key.n:
        raise ValueError("Message is too large for the RSA key modulus.")

    # 2. Apply the RSA private key operation (m^d mod n)
    # This is what a signature does, but normally to a HASH, not the raw message.
    encrypted_int = pow(message_as_int, private_key.d, private_key.n)

    # 3. Convert the resulting integer back to bytes
    # This is now a reversible block, NOT a secure signature.
    reversible_block = long_to_bytes(encrypted_int, private_key.size_in_bytes())

    print("Generated reversible block (insecure 'signature').")

    # We still return the public key components for the CTF solver
    public_key = private_key.publickey()
    modulus = public_key.n
    exponent = public_key.e

    return reversible_block, (modulus, exponent)
    
def generate_flag() -> bytes:
    key = os.urandom(32)
    iv = os.urandom(16)

    with open('flag.txt', 'rb') as fp:
        flag = fp.read()
    
    cipher = pyaes.new(key, pyaes.MODE_CBC, iv)
    padded_flag = pad(flag, pyaes.block_size)
    ciphertext = cipher.encrypt(padded_flag)

    key_material = key + b'\0\0' + iv
    sig, _ = generate_reversible_block(key_material)

    # Combine using a length-prefix format
    len_c = struct.pack('>I', len(ciphertext))
    len_s = struct.pack('>I', len(sig))
    
    enc_flag = len_c + len_s + ciphertext + sig

    return enc_flag
    
# --- Main Execution ---

def main() -> None:
    # Create an instance of the compressor to manage C library resources safely
    compressor = LZOCompressor()

    print("--- BUILDING STAGE 1 FLASH ---")
    build_stage1_flash()

    print("\n--- BUILDING STAGE 2 FLASH ---")
    build_stage2_flash()

    print("\n--- GENERATING SIGNATURE FOR STAGE 1 ---")
    with open("../flash_rom_stage1/bin/flashrom1.bin", 'rb') as fp:
        data1 = fp.read()
        sig1, (modulus, exponent) = generate_sig_blocks(data1)
    
    print(f"Signature1: {sig1} ({len(sig1)})")
    print(f"Modulus: {modulus}")
    print(f"Exponent: {exponent}")

    print("\n--- GENERATING SIGNATURE FOR STAGE 2 ---")
    with open("../flash_rom_stage2/bin/flashrom2.bin", 'rb') as fp:
        data2 = fp.read()
        sig2, (modulus, exponent) = generate_sig_blocks(data2)
    
    print(f"Signature2: {sig2} ({len(sig2)})")
    print(f"Modulus: {modulus}")
    print(f"Exponent: {exponent}")

    print("\n--- COMPRESSING STAGE 2 ---")
    print(f"Uncompressed size: {len(data2)}")
    compressed_data = compressor.compress(data2)

    print("\n--- ENCRYPTING STAGE 2 ---")
    encrypted_data, key, iv = encrypt_flash_img(compressed_data)
    
    print(f"Key: {key}")
    print(f"IV: {iv}")
    
    print("\n--- PREPARING FINAL FLASH IMAGE ---")
    prepare_flash_img_user(data1, sig1, encrypted_data, sig2)
    flag = generate_flag()
    prepare_flash_img_flag(data1, sig1, encrypted_data, sig2, flag)


if __name__ == "__main__":
    main()