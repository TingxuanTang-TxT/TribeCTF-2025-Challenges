package main

import (
	"bufio"
	"crypto/sha256"
	"encoding/hex"
	"flag"
	"fmt"
	"os"
	"strconv"

	"rngcrack/internal/aes256"
	"rngcrack/internal/mu"
	"rngcrack/lcg"
)

const usage = `Usage: setup [options] RNG_FILE ENC_FILE FLAG

Create and RNG, writing the first N random values to the RNG_FILE
and encrypting FLAG to the output ENC_FILE using the N+1 random value.

options:
  -modulus M
    The LCG's modulus

  -multiplier A
    The LCG's multiplier

  -increment C
    The LCG's increment

  -num-values N
    The number of random numbers to write to RNG_FILE
`

const hexIV = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

type Options struct {
	// positional
	rngFile string
	encFile string
	flag    string
	// options
	seed       int
	modulus    int
	multiplier int
	increment  int
	numValues  int
}

func printUsage() {
	fmt.Fprintf(os.Stderr, "%s", usage)
}

func parseOptions() *Options {
	options := Options{}

	flag.Usage = printUsage
	flag.IntVar(&options.seed, "seed", 78, "")
	flag.IntVar(&options.modulus, "modulus", 2147483647, "") // 1<<31-1
	flag.IntVar(&options.multiplier, "multiplier", 50423, "")
	flag.IntVar(&options.increment, "increment", 4253, "")
	flag.IntVar(&options.numValues, "num-values", 1000, "")

	flag.Parse()

	if flag.NArg() != 3 {
		mu.Fatalf("expected three positional argument but got %d", flag.NArg())
	}

	options.rngFile = flag.Arg(0)
	options.encFile = flag.Arg(1)
	options.flag = flag.Arg(2)

	return &options
}

func main() {
	options := parseOptions()

	f, err := os.OpenFile(options.rngFile, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0o644)
	if err != nil {
		mu.Fatalf("failed to create rng file: %v", err)
	}

	w := bufio.NewWriter(f)
	defer func() {
		_ = w.Flush()
		_ = f.Close()

	}()

	myLCG := lcg.NewLCG(options.multiplier, options.increment, options.modulus, options.seed)
	for i := 0; i < options.numValues; i++ {
		val := myLCG.Generate()
		if _, err := fmt.Fprintln(w, val); err != nil {
			mu.Fatalf("error writing to rng file: %v", err)
		}
	}

	val := myLCG.Generate()
	aesKey := sha256.Sum256([]byte(strconv.Itoa(val)))
	iv, err := hex.DecodeString(hexIV)
	if err != nil {
		mu.BUG("failed to unhexilfy iv: %v", err)
	}

	ct := aes256.EncryptCTR(aesKey[:], iv, []byte(options.flag))

	if err := os.WriteFile(options.encFile, ct, 0o644); err != nil {
		mu.Fatalf("failed to write ciphertext file: %v", err)
	}
}
