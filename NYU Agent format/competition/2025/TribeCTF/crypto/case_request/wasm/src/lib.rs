#![no_std]
use chacha20::cipher::{KeyIvInit, StreamCipher};
use core::{arch::wasm32, cell::LazyCell, ffi::c_void};

#[panic_handler]
fn handle_panic(_: &core::panic::PanicInfo) -> ! {
    core::arch::wasm32::unreachable()
}
const _: () = {
    assert!(core::mem::size_of::<usize>() == core::mem::size_of::<i32>());
    assert!(core::mem::size_of::<*const ()>() == core::mem::size_of::<i32>());
};

static mut KEYS: [[u8; 32]; 16] = [[0; 32]; 16];
static mut FILLED: u8 = 0;
static mut POSITION: u8 = 0;
static mut DECRYPTED_ACTUAL: bool = false;

struct KeyHandle {
    pub inner: u32,
}

const MEMORY_LEN: usize = 64 * 1024 * 100;
// Linear memory starting at 0
const MEMORY_BASE: *const u8 = 0 as *const u8;
unsafe extern "C" {
    pub unsafe fn fill_random(buffer: usize, n: i32);
}

static mut OFFSET: usize = 0;
static mut XOR_KEY: LazyCell<u32> = LazyCell::new(|| {
    let data_ptr = alloc(4);
    let data = unsafe { core::slice::from_raw_parts_mut(data_ptr as *mut u8, 4) };
    fill_buffer_random(&mut data[..]);
    let res = u32::from_le_bytes(data.try_into().unwrap());
    dealloc(data_ptr);
    res
});

#[unsafe(no_mangle)]
pub extern "C" fn alloc(size: usize) -> usize {
    if unsafe { OFFSET + size } > MEMORY_LEN {
        panic!();
    }
    let ptr = unsafe { OFFSET };
    unsafe { OFFSET += size };
    return ptr;
}

#[unsafe(no_mangle)]
pub extern "C" fn dealloc(ptr: usize) {
    unsafe { OFFSET = ptr as usize }
}

fn fill_buffer_random(buffer: &mut [u8]) {
    unsafe { fill_random(buffer.as_ptr() as usize, buffer.len() as i32) };
}

fn i_to_u(i: i32) -> u32 {
    u32::from_le_bytes(i.to_le_bytes())
}

fn u_to_i(u: u32) -> i32 {
    i32::from_le_bytes(u.to_le_bytes())
}

impl KeyHandle {
    const PRIME: u32 = 395220835;
    const INV: u32 = 3639583307;
    fn serialize(self) -> i32 {
        let result = u_to_i(self.inner.wrapping_mul(Self::PRIME) ^ unsafe { *XOR_KEY });
        result
    }
    fn deserialize(outer: u32) -> Self {
        Self {
            inner: (outer ^ unsafe { *XOR_KEY }).wrapping_mul(Self::INV),
        }
    }

    fn add_key() -> i32 {
        unsafe {
            #[allow(static_mut_refs)]
            let keys_len = const { KEYS.len() };
            if FILLED < keys_len as u8 {
                POSITION = FILLED;
                FILLED += 1;
            } else {
                POSITION += 1;
                POSITION %= keys_len as u8;
            }

            let mut buffer = KEYS[POSITION as usize];
            fill_buffer_random(&mut buffer);

            KeyHandle {
                inner: POSITION as u32,
            }
            .serialize()
        }
    }

    fn get_key(handle: i32) -> [u8; 32] {
        let key_index = Self::deserialize(i_to_u(handle)).inner;

        if key_index < 16 {
            let key = unsafe { KEYS[key_index as usize].clone() };
            key
        } else {
            wasm32::unreachable()
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn crc32(data: usize, len: i32) -> i32 {
    let data_slice = unsafe { core::slice::from_raw_parts(MEMORY_BASE.add(data), len as usize) };
    let crc = zlib_rs::crc32(0, data_slice);
    u_to_i(crc)
}

#[unsafe(no_mangle)]
pub extern "C" fn get_key_handle() -> i32 {
    KeyHandle::add_key()
}

fn check_decrypt(data: &[u8]) -> bool {
    if core::hint::black_box(const { check_decrypt as *const u8 }) == check_decrypt as *const u8 {
        enum State {
            Nothing,
            ApprovedFound,
            ColonFound,
        }
        let mut data = data;
        let mut state = State::Nothing;
        while !data.is_empty() {
            match state {
                State::Nothing => {
                    if let Some(new_data) = data.strip_prefix(b"\"approved\"") {
                        state = State::ApprovedFound;
                        data = new_data;
                    } else {
                        data = &data[1..];
                    }
                }
                State::ApprovedFound => match data[0] {
                    b' ' | b'\t' | b'\n' | b'\r' => {
                        data = &data[1..];
                    }
                    b':' => {
                        state = State::ColonFound;
                        data = &data[1..];
                    }
                    _ => {
                        state = State::Nothing;
                    }
                },
                State::ColonFound => {
                    if data.starts_with(b"true") {
                        return true;
                    } else if b" \n\r\t".contains(&data[0]) {
                        data = &data[1..];
                    } else {
                        state = State::Nothing;
                    }
                }
            }
        }
        false
    } else {
        wasm32::unreachable()
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn get_flag() -> *const c_void {
    if core::hint::black_box(const { get_flag as *const u8 }) == get_flag as *const u8 {
        const FLAG: &[u8] = include_bytes!("flag.txt");
        let flag_buf = const {
            let mut result = [0; FLAG.len() + 1];
            let mut i = 0;
            while i < FLAG.len() {
                result[i] = !(i as u8) ^ FLAG[i];
                i += 1;
            }
            result
        };
        let dec = unsafe { DECRYPTED_ACTUAL };

        if dec {
            let new_buf = alloc(FLAG.len() + 1);
            let new_buf: &mut [u8] =
                unsafe { core::slice::from_raw_parts_mut(new_buf as *mut u8, FLAG.len() + 1) };

            for (i, c) in flag_buf.iter().take(FLAG.len()).enumerate() {
                new_buf[i] = !(i as u8) ^ c;
            }
            new_buf[FLAG.len()] = 0;

            new_buf.as_ptr() as *const c_void
        } else {
            const ERR: &[u8; 31] = b"your case has not been accepted";
            let buf = alloc(ERR.len() + 1);
            let slice = unsafe { core::slice::from_raw_parts_mut(buf as *mut u8, ERR.len() + 1) };
            slice[..31].copy_from_slice(ERR);
            slice[31] = 0;
            buf as *const c_void
        }
    } else {
        wasm32::unreachable()
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn chacha20_encrypt(data: usize, length: i32, nonce: usize, key_handle: i32) -> i32 {
    let key = KeyHandle::get_key(key_handle);

    let slice = unsafe {
        core::slice::from_raw_parts_mut(MEMORY_BASE.add(data) as *mut u8, length as usize)
    };

    let nonce = unsafe { core::slice::from_raw_parts(MEMORY_BASE.add(nonce), 12) };

    chacha20::ChaCha20::new(&key.into(), nonce.into()).apply_keystream(slice);

    // if the decrypted string contains "actual": true, then they have solved the challenge
    if check_decrypt(slice) {
        unsafe {
            DECRYPTED_ACTUAL = true;
        }
    }

    data as i32
}
