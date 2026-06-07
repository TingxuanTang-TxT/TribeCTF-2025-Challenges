# Sir Wren Summons!

## Difficulty
- **Easy**

## Category
- **Forensics**

## Description
This is a simple steganography challenge. The flag is hidden in the image header,
which is revealed when one goes through the hexdump. 

## Solution
```bash
xxd sir_wren_summons.jpg >> hex
cat hex
```
```bash
000000c0: 0000 0405 0203 0601 0007 08ff fe00 3264  ..............2d
000000d0: 484a 7059 6d56 4456 455a 3749 6c6c 7664  HJpYmVDVEZ7Illvd
000000e0: 5639 6f59 585a 6c58 3246 7963 6d6c 325a  V9oYXZlX2Fycml2Z
000000f0: 5752 6659 5852 664d 5459 354d 794a 39ff  WRfYXRfMTY5MyJ9.
```
Plug the string starting with `dHJ...` to a base64 decoder, and the flag is
revealed: `tribeCTF{hack_upon_the_flag}`
