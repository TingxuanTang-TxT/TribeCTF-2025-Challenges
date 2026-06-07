mindpq
============================================================

Flag
------------------------------------------------------------

```
tribectf{Decision making, like coffee, needs a cooling process - GW}
```

Overview
------------------------------------------------------------
This challenge involves finding a two RSA keys that share a common factor.  The
challenges have 100 RSA keys.  At a high-level, determining a shared factor
simply involves taking each pair of keys and computing the GCD; if the GCD is
not 1, then they have a common factor, say `p`.  Once you know `p`, it is
trivial to divide the modulus `n` by `p` to determine `q.

The attack that is implemented here uses an optimized search for a common
factor.  However, with only 100 keys, it is perfectly feasible for a team to
perform the O(N^2) operation of pairwise-comparing all keys.

For the challenge, I lightly adapted the " Common-factors attack and the effect
of poor random number generation on cryptographic security" example from
Section 8.4.1 of the book:

```
@book{25-hacking-cryptography,
  title     = "Hacking Cryptographyt"
  author    = "Khan, Kamran and Cox, Bill"
  year      = 2025,
  publisher = "Manning Publications,
  address   = "Shelter Island, NY"
}
```

For the challenge, the team receives a zip file that contains two files 100
ciphertext files `ct-i.bin` and 100 PEM-encoded public RSA keys (`key-i.pem`),
where `key-i` encrypts `ct-i`.  The goal is to find the two keys that have a
common factor and decrypt their corresponding `ct-i.bin`.  The flag is then the
concatenation of the two resultant plaintexts.


Setting Up the Challenge
------------------------------------------------------------
The zip file `mindpq.zip` in this repo already contains
the challenge.  It was generated as follows:

```
make mindpq 'tribectf{Decision making, like coffee, needs a cooling process - GW}'
zip -r mindpq.zip mindpq
```

To solve the challenge, simply run

```
./explit mindpq
```
