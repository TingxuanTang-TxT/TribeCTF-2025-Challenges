package attack

import (
	cryptoRand "crypto/rand"
	"crypto/rsa"
	"testing"

	"mindpq/rsautil"
)

const (
	TotalKeypairs = 48
)

func TestCommonFactorsAttack(t *testing.T) {
	t.Logf("generating %d keypairs using a pool of %d primes", TotalKeypairs, rsautil.PrimesPoolTotal)

	type testCase struct {
		pubKey     *rsa.PublicKey
		ciphertext []byte
	}
	var testCases []testCase
	var pubKeys []*rsa.PublicKey

	for i := 0; i < TotalKeypairs; i++ {
		pubKey, ciphertext := rsautil.GeneratePublicKeyAndCiphertext()
		testCases = append(testCases, testCase{
			pubKey,
			ciphertext,
		})
		pubKeys = append(pubKeys, pubKey)
	}

	recoveredPrivKeys, _ := RecoverPrivateKeysUsingCommonFactors(pubKeys)

	if len(recoveredPrivKeys) == 0 {
		t.Fatalf("could not recover any private keys")
	}

	t.Logf("recovered %d private keys from %d public keys", len(recoveredPrivKeys), len(pubKeys))

	for _, testCase := range testCases {
		for _, privKey := range recoveredPrivKeys {
			if testCase.pubKey.E != privKey.E || testCase.pubKey.N.Cmp(privKey.N) != 0 {
				continue
			}

			decrypted, err := rsa.DecryptPKCS1v15(cryptoRand.Reader, privKey, testCase.ciphertext)
			if err != nil {
				t.Fatalf("error decrypting: %s", err)
			}

			t.Logf("decrypted: %s", decrypted)
		}
	}
}
