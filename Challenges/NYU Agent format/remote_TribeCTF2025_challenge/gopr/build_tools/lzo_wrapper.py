import ctypes
from ctypes import c_int, c_size_t, c_void_p, c_uint8, POINTER, create_string_buffer

# Load the compiled shared object
lzo = ctypes.CDLL('./crypto_libs/liblzo.so')

# Function prototypes
lzo.py_lzo_init.restype = c_int

lzo.py_lzo_compress.argtypes = [
    POINTER(c_uint8), c_size_t,
    POINTER(c_uint8), POINTER(c_size_t),
    c_void_p
]
lzo.py_lzo_compress.restype = c_int

lzo.py_lzo_decompress.argtypes = [
    POINTER(c_uint8), c_size_t,
    POINTER(c_uint8), POINTER(c_size_t)
]
lzo.py_lzo_decompress.restype = c_int

# Init
if lzo.py_lzo_init() != 0:
    raise RuntimeError("LZO init failed")

# Data to compress
data = b"this is some test data to compress" * 10
in_len = len(data)

# Estimate worst-case output size
out_max = in_len + in_len // 16 + 64 + 3

# Allocate buffers
in_buf = create_string_buffer(data, in_len)
out_buf = create_string_buffer(out_max)
workmem = ctypes.create_string_buffer(16384)  # LZO1X_1_MEM_COMPRESS

out_len = c_size_t(out_max)

# Compress
res = lzo.py_lzo_compress(
    ctypes.cast(in_buf, POINTER(c_uint8)), in_len,
    ctypes.cast(out_buf, POINTER(c_uint8)), ctypes.byref(out_len),
    ctypes.cast(workmem, c_void_p)
)
if res != 0:
    raise RuntimeError(f"Compression failed with code {res}")

print(f"Compressed size: {out_len.value}")

# Decompress
decomp_buf = create_string_buffer(in_len)
decomp_len = c_size_t(in_len)

res = lzo.py_lzo_decompress(
    ctypes.cast(out_buf, POINTER(c_uint8)), out_len.value,
    ctypes.cast(decomp_buf, POINTER(c_uint8)), ctypes.byref(decomp_len)
)
if res != 0:
    raise RuntimeError(f"Decompression failed with code {res}")

# Verify
if decomp_buf.raw[:decomp_len.value] == data:
    print("Compression and decompression successful!")
else:
    print("Mismatch after decompression!")
