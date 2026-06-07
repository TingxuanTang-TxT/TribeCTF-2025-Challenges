import asyncio
import subprocess
import tempfile
import shutil
import os
import signal

# --- Paths updated for the Docker container environment ---
# The QEMU binary is copied to this location by the Dockerfile.
QEMU_BINARY = "/qemu/build/qemu-system-arm"
# The assets are mounted into the container via docker-compose.yml.
BASE_FLASH = "/app/assets/flash_server.img"
BOOTLOADER = "/app/assets/bootloader.bin"
SHADOW_BOOTLOADER = "/app/assets/shadow.bin"
TIMEOUT = 600  # 10 minutes in seconds

async def handle_client(reader, writer):
    addr = writer.get_extra_info('peername')
    print(f"[+] Connection from {addr}")

    # Create isolated temp flash image for this session
    tmp_dir = tempfile.mkdtemp(prefix="qemu_instance_")
    tmp_flash = os.path.join(tmp_dir, "flash.img")

    try:
        shutil.copy(BASE_FLASH, tmp_flash)
    except FileNotFoundError:
        print(f"[!] ERROR: Base flash image not found at {BASE_FLASH}. Make sure assets are mounted correctly.")
        writer.close()
        await writer.wait_closed()
        shutil.rmtree(tmp_dir)
        return

    qemu_cmd = [
        QEMU_BINARY,
        "-M", "tribe",
        "-cpu", "cortex-r5",
        "-nographic",
        # Redirect serial to stdio so we can pipe it over the network
        "-serial", "mon:stdio",
        "-drive", f"file={tmp_flash},if=none,format=raw,id=flash0",
        "-device", f"loader,file={BOOTLOADER},addr=0x0",
        "-device", f"loader,file={SHADOW_BOOTLOADER},addr=0x00100000",
        # Crucially, disable the monitor and networking for security
        "-monitor", "none",
        "-net", "none"
    ]

    proc = await asyncio.create_subprocess_exec(
        *qemu_cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )

    async def forward_qemu_output():
        try:
            while True:
                data = await proc.stdout.read(1024)
                if not data:
                    break
                writer.write(data)
                await writer.drain()
        except (asyncio.CancelledError, ConnectionResetError):
            pass
        finally:
            writer.close()

    async def forward_client_input():
        try:
            while True:
                try:
                    data = await asyncio.wait_for(reader.read(1024), timeout=TIMEOUT)
                except asyncio.TimeoutError:
                    print(f"[!] Connection from {addr} timed out")
                    break
                if not data:
                    break
                proc.stdin.write(data)
                await proc.stdin.drain()
        except (asyncio.CancelledError, ConnectionResetError):
            pass

    tasks = [
        asyncio.create_task(forward_qemu_output()),
        asyncio.create_task(forward_client_input())
    ]

    try:
        await asyncio.wait(tasks, return_when=asyncio.FIRST_COMPLETED)
    finally:
        for t in tasks:
            t.cancel()
        if proc.returncode is None:
            # Cleanly terminate the QEMU subprocess
            try:
                proc.send_signal(signal.SIGTERM)
                await asyncio.wait_for(proc.wait(), timeout=5.0)
            except asyncio.TimeoutError:
                print(f"[!] QEMU process for {addr} did not terminate gracefully, killing.")
                proc.kill()
                await proc.wait()
        
        if writer.can_write_eof() and not writer.is_closing():
            writer.write_eof()
        writer.close()
        await writer.wait_closed()
        
        shutil.rmtree(tmp_dir)
        print(f"[-] Connection from {addr} closed and QEMU instance cleaned up")

async def main(host="0.0.0.0", port=4000):
    server = await asyncio.start_server(handle_client, host, port)
    addr = server.sockets[0].getsockname()
    print(f"[+] Listening for connections on {addr}")
    async with server:
        await server.serve_forever()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n[+] Server shutting down.")