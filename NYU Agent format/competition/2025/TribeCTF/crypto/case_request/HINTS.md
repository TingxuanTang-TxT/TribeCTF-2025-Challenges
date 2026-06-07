You have access to the code of the site, so you can make changes to print
things etc


How does ChaCha20 encrypt data? Does it prevent the data from being modified?
(It's a stream cipher, so you can change known parts of the plaintext at will)

What properties does CRC32 have? (see the wikipedia page)
(It's an affine transform, so crc32(old ^ diff) = crc32(old) ^ crc32(diff) ^ crc32(zeros))

If you tried to patch the code to call the flag function:
It won't actually print the flag unless you first decrypt a valid token
that would solve the challenge. 

