package main

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"mindpq/internal/mu"
	"mindpq/rsautil"
)

const usage = `Usage: setup OUTPUT_DIR FLAG

Setup the challenge artifacts.

options:
  -help
    Display this help message.

example:
  ./setup artifacts 'tribectf{foo bar baz}'
`

type Options struct {
	outputDir string
	ctfFlag   string
}

func printUsage() {
	fmt.Fprintf(os.Stderr, "%s", usage)
}

func parseOptions() *Options {
	options := Options{}

	flag.Usage = printUsage

	flag.Parse()

	if flag.NArg() != 2 {
		mu.Fatalf("expected two positional arguments but got %d", flag.NArg())
	}

	options.outputDir = flag.Arg(0)
	options.ctfFlag = flag.Arg(1)

	if len(options.ctfFlag) > rsautil.MaxPlaintextSize {
		mu.Fatalf("ctf flag is too long; must be < %d", rsautil.MaxPlaintextSize)
	}

	return &options
}

func createOutputDir(path string) {
	if _, err := os.Stat(path); os.IsNotExist(err) {
		// Does not exist, so create it
		err := os.Mkdir(path, 0755)
		if err != nil {
			mu.Fatalf("failed to create output directory: %v", err)
		}
	}
}

func splitFlag(ctfFlag string) (string, string) {
	n := len(ctfFlag)
	return ctfFlag[:n/2], ctfFlag[n/2:]
}

func main() {
	options := parseOptions()

	createOutputDir(options.outputDir)

	firstHalfFlag, secondHalfFlag := splitFlag(options.ctfFlag)
	privKeys := rsautil.GenerateChallengeKeys()
	for i, sk := range privKeys {
		pk := &sk.PublicKey
		var msg string
		if i == rsautil.CommonFactorKeyA {
			msg = firstHalfFlag
		} else if i == rsautil.CommonFactorKeyB {
			msg = secondHalfFlag
		} else {
			msg = strings.Repeat(string(rune(i)), rsautil.MaxPlaintextSize)
		}
		ct := rsautil.Encrypt(pk, []byte(msg))
		err := os.WriteFile(filepath.Join(options.outputDir, fmt.Sprintf("ct-%0d.bin", i)), ct, 0o644)
		if err != nil {
			mu.Fatalf("failed to create ciphertext file: %v", err)
		}
		err = rsautil.StorePublicKeyToPEMFile(pk, filepath.Join(options.outputDir, fmt.Sprintf("key-%02d.pem", i)))
		if err != nil {
			mu.Fatalf("failed to create public key file: %v", err)
		}
	}

}
