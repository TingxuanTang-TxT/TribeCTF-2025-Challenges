package aes256

import (
	"crypto/aes"
	"crypto/cipher"

	"rngcrack/internal/mu"
)

const KeySize = 32

// NewGTR creates a [cipher.Stream] for AES-256 CTR mode.
func NewCTR(key []byte, iv []byte) cipher.Stream {
	if len(key) != KeySize {
		mu.Panicf("aes256.NewCTR: bad key size %d", len(key))
	}

	block, err := aes.NewCipher(key)
	if err != nil {
		mu.Panicf("aes256.NewCTR: aes.NewCipher failed: %v", err)
	}

	return cipher.NewCTR(block, iv)
}

// DoCTR performs a one-shot AES-256 CTR operation on data. The function reuses
// the data slice for the output.  As a convenience, this function also returns
// the output slice.
func DoCTR(key, iv, data []byte) []byte {
	aesctr := NewCTR(key, iv)
	aesctr.XORKeyStream(data, data)
	return data
}

// EncryptCTR performs a one-shot AES-256 CTR encryption of the plaintext data.
// This function reuses the data slice for the ciphertext.  Thus, on return,
// the plaintext is ovewritten with the ciphertext.  As a convenience, this
// function also returns the output slice.
func EncryptCTR(key, iv, data []byte) []byte {
	return DoCTR(key, iv, data)
}

// EncryptCTR performs a one-shot AES-256 CTR decryption of the ciphertext
// data.  This function reuses the data slice for the plaintext.  Thus, on
// return, the ciphertext is ovewritten with the plaintext.  As a convenience,
// this function also returns the slice.
func DecryptCTR(key, iv, data []byte) []byte {
	return DoCTR(key, iv, data)
}
