package attack

import (
	"testing"
	"time"

	"rngcrack/lcg"
)

func TestCrackLCG(t *testing.T) {
	multiplier := 48271
	increment := 0
	modulus := 1<<31 - 1
	seed := time.Now().UnixNano()

	originalRng := lcg.NewLCG(multiplier, increment, modulus, int(seed))
	var pastValues []int
	for i := 0; i < 1000; i++ {
		pastValues = append(pastValues, originalRng.Generate())
	}

	clonedRng := CrackLCG(pastValues)

	for i := 0; i < 100; i++ {
		clonedValue := uint32(clonedRng.Generate())
		observedValue := uint32(originalRng.Generate())
		if observedValue != clonedValue {
			t.Fatalf("observed: %08x, cloned: %08x", clonedValue, observedValue)
		}
		if i%20 == 0 {
			t.Logf("observed: %08x, cloned: %08x", clonedValue, observedValue)
		}
	}
}
