rngcrack
============================================================

Flag
------------------------------------------------------------

```
tribectf{I cannot live without books and CSPRNGs. - Thomas Jefferson, maybe}
```

Overview
------------------------------------------------------------
This challenge is cracking a (non-cryptographic) random
number generator (RNG), and specifically one implemented as a
linear congruential generator (LCG).  An LCG is a function
of the form:

```
x_{n+1} = (a * x_n + c) mod m
```

where,

- `x_n` is the previous random number
- `x_{n+1} is the next random number
- `a` is the multiplier
- `c' is the increment
- `m` is the modulus


For the challenge, I lightly adapted the "Exploiting LCGs"
example from Section 2.2 of the book:

```
@book{25-hacking-cryptography,
  title     = "Hacking Cryptographyt"
  author    = "Khan, Kamran and Cox, Bill"
  year      = 2025,
  publisher = "Manning Publications,
  address   = "Shelter Island, NY"
}
```

For the challenge, the team receives a zip file that
contains two files:

- `stream.dat`
    A list of the first 1000 numbers the RNG generated

- `ciphertext.bin`
    A ciphertext file, where the plaintext is the flag.  The
    file is encrypted with AES-256 in CTR mode.  The key is the
    SHA256 hash of the 1001st RNG value (i.e., the next
    value the RNG would generate).  Specifically, the key
    is `SHA256(string(rng_value))`

The goal of the challenge is to use stream.dat to clone the
LCG (i.e., determine its internal parameters `a`, `c`, and
`m`), and thus produce the 1001st RNG value.


Setting Up the Challenge
------------------------------------------------------------
The zip file `rngcrack.zip` in this repo already contains
the challenge.  It was generated as follows:

```
$ make
$ ./setup stream.dat ciphertext.dat 'tribectf{I cannot live without books and CSPRNGs. - Thomas Jefferson, maybe}'
$ mkdir rngcrack
$ mv stream.dat ciphertext.dat rngcrack
$ zip -r rngcrack.zip rngcrack
```

If you want to crack this, enter:

```
./exploit rngcrack/stream.dat rngcrack/ciphertext.bin 
```

Note that `setup` takes options for changing the LCG parameters and flag (you
need to include the "tribectf{}" part); we simply use its defaults.
