package rsautil

import (
	cryptoRand "crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"encoding/pem"
	"errors"
	"math/big"
	mathRand "math/rand"
	"os"
	"time"

	"mindpq/internal/mu"
)

const (
	ModulusBits      = 1024
	PrimesPoolTotal  = 512
	MaxPlaintextSize = 117
	CommonFactorKeyA = 31
	CommonFactorKeyB = 72
)

var (
	PrimesPool [PrimesPoolTotal]*big.Int
)

// init creates a pool of 512 primes, each 512-bits long
func init() {
	var prev *big.Int

	i := 0
	for i < PrimesPoolTotal {
		p := GeneratePrime(ModulusBits / 2)

		// ensure adjacent primes in the pool are different
		if prev == nil {
			PrimesPool[i] = p
		} else if prev.Cmp(p) != 0 {
			PrimesPool[i] = p
		} else {
			continue
		}
		i += 1
		prev = p
	}
}

// GenerateRSAPrivateKeyUsingChilledRNG generates and returns an
// *rsa.PrivateKey using the pool of primes (what we called the "Chilled
// RNG").
func GeneratePrivateKeyUsingChilledRng() *rsa.PrivateKey {
	rng := mathRand.New(mathRand.NewSource(time.Now().UnixMicro()))
	var p, q *big.Int
	for {
		p = PrimesPool[rng.Intn(PrimesPoolTotal)]
		q = PrimesPool[rng.Intn(PrimesPoolTotal)]
		if p != q {
			break
		}
	}

	return GeneratePrivateKeyFromFactors(p, q)
}

// GenerateRSAPublicKeyAndCiphertext creates an RSA keypair, discards the
// private key, and returns a ciphertext encrypted to that public key.
func GeneratePublicKeyAndCiphertext() (*rsa.PublicKey, []byte) {
	privKey := GeneratePrivateKeyUsingChilledRng()

	pubKey := &rsa.PublicKey{
		N: privKey.N,
		E: privKey.E,
	}

	message := time.Now().String()

	ciphertext, err := rsa.EncryptPKCS1v15(cryptoRand.Reader, pubKey, []byte(message))
	if err != nil {
		mu.Fatalf("rsa.EcnryptPKCS1v15 failed: %v", err)
	}

	return &privKey.PublicKey, ciphertext
}

/////////////////////////////////////////////////////////////////////////

func GeneratePrime(bits int) *big.Int {
	p, err := cryptoRand.Prime(cryptoRand.Reader, bits)
	if err != nil {
		mu.Panicf("crypto/rand Prime() failed: %v", err)
	}
	return p
}

func GeneratePrivateKeyFromFactors(p, q *big.Int) *rsa.PrivateKey {
	if p.Cmp(q) == 0 {
		mu.BUG("trying to generate RSA private key with p == q")
	}

	pMinus1 := new(big.Int).Sub(p, big.NewInt(1))
	qMinus1 := new(big.Int).Sub(q, big.NewInt(1))
	modulus := new(big.Int).Mul(p, q)
	phi := new(big.Int).Mul(pMinus1, qMinus1)

	var err error
	e := new(big.Int)
	for {
		e, err = cryptoRand.Int(cryptoRand.Reader, big.NewInt(1<<31-1))
		if err != nil {
			mu.Fatalf("crypto/rand failed: %v", err)
		}
		egcd := new(big.Int).GCD(nil, nil, e, phi)
		if egcd.Int64() == 1 {
			break
		}
	}

	d := new(big.Int).ModInverse(e, phi)

	pk := &rsa.PublicKey{
		N: modulus,
		E: int(e.Int64()),
	}

	sk := &rsa.PrivateKey{
		PublicKey: *pk,
		D:         d,
		Primes:    []*big.Int{p, q},
	}

	err = sk.Validate()
	if err != nil {
		mu.Fatalf("private RSA key failed validation: %v", err)
	}

	return sk
}

func GenerateChallengeKeys() []*rsa.PrivateKey {
	privKeys := make([]*rsa.PrivateKey, 0, 100)
	aFactorP := new(big.Int)

	i := 0
	for i < 100 {
		p := GeneratePrime(ModulusBits / 2)
		q := GeneratePrime(ModulusBits / 2)
		if p.Cmp(q) == 0 {
			continue
		}

		if i == CommonFactorKeyA {
			aFactorP = new(big.Int).Set(p)
		}

		if i == CommonFactorKeyB {
			q = aFactorP
			if p.Cmp(q) == 0 {
				continue
			}
		}

		sk := GeneratePrivateKeyFromFactors(p, q)
		privKeys = append(privKeys, sk)

		i++
	}
	return privKeys
}

func MarshalPublicKeyToPEM(pk *rsa.PublicKey) []byte {
	derData := x509.MarshalPKCS1PublicKey(pk)

	block := &pem.Block{
		Type:  "RSA PUBLIC KEY",
		Bytes: derData,
	}

	pemData := pem.EncodeToMemory(block)
	if pemData == nil {
		mu.Fatalf("failed to PEM encode RSA public key")
	}

	return pemData
}

func StorePublicKeyToPEMFile(pk *rsa.PublicKey, keyPath string) error {
	pemData := MarshalPublicKeyToPEM(pk)
	return os.WriteFile(keyPath, pemData, 0o644)
}

func UnmarshalPublicKeyFromPEM(pemData []byte) (*rsa.PublicKey, error) {
	block, _ := pem.Decode([]byte(pemData))
	if block == nil {
		return nil, errors.New("error: failed fo parse PEM block containing public key")
	}

	pk, err := x509.ParsePKCS1PublicKey(block.Bytes)
	if err != nil {
		return nil, err
	}

	return pk, nil
}

func LoadPublicKeyFromPEMFile(keyPath string) (*rsa.PublicKey, error) {
	pemData, err := os.ReadFile(keyPath)
	if err != nil {
		return nil, err
	}
	return UnmarshalPublicKeyFromPEM(pemData)
}

func Encrypt(pk *rsa.PublicKey, msg []byte) []byte {
	ciphertext, err := rsa.EncryptPKCS1v15(cryptoRand.Reader, pk, msg)
	if err != nil {
		mu.Panicf("failed to RSA encrypt: %v", err)
	}
	return ciphertext
}

func Decrypt(sk *rsa.PrivateKey, ciphertext []byte) ([]byte, error) {
	return rsa.DecryptPKCS1v15(cryptoRand.Reader, sk, ciphertext)
}
