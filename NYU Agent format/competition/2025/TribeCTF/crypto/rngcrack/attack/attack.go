package attack

import (
	"math/big"

	"rngcrack/lcg"
)

func findModInverse(a, m int64) int64 {
	return new(big.Int).ModInverse(big.NewInt(a), big.NewInt(m)).Int64()
}

func findGCD(a, b int64) int64 {
	return new(big.Int).GCD(nil, nil, big.NewInt(a), big.NewInt(b)).Int64()
}

func findModulus(pastValues []int) int {
	var diffs []int
	previousValue := pastValues[0]
	for i := 1; i < len(pastValues); i++ {
		currentValue := pastValues[i]
		diffs = append(diffs, currentValue-previousValue)
		previousValue = currentValue
	}

	var zeros []int
	for i := 2; i < len(diffs); i++ {
		zeros = append(zeros, diffs[i]*diffs[i-2]-diffs[i-1]*diffs[i-1])
	}

	gcd := 0
	for _, v := range zeros {
		gcd = int(findGCD(int64(gcd), int64(v)))
	}

	return gcd
}

// findMultiplier assumes the attacker knows the modulus m,
// but not the multiplier a.
//
// This function solves for a based on the equations:
//
//	x1 = (a*x0 + c) mod m
//	x2 = (a*x1 + c) mod m
//
//	=>
//
//	x2 - x1 = (a*x1 - a*x0) mod m   =>
//	x2 - x1 = (a * (x1 - x0)) mod m
//
// Thus, we have
//
//	a = ((x2 - x1) / (x1 - x0)) mod m
//
// The only complication is that we can't directly input the x values for the
// denominator, but must instead find the modular multiplicative inverse of the
// value (x1 - x0).
func findMultiplier(modulus, s0, s1, s2 int) int {
	inverse := int(findModInverse(int64(s1-s0), int64(modulus)))

	// N.B. Go differs from other programming languages in terms of the
	// behavior of its modulus operation.  In most programming languages,
	// -16 % 7 is 5, but Go returns -2.
	multiplier := (s2 - s1) * inverse % modulus
	if multiplier < 0 {
		return modulus + multiplier
	} else {
		return multiplier
	}
}

// findIncrement assumes the attacker knows the muliplier a and the modulus m
// but not the increment c.  It solves for c in the equation:
//
//	x_n = a*x_{n-1} + c mod m
//
// TODO: change this so it's always positive
func findIncrement(modulus, multiplier, s0, s1 int) int {
	increment := (s1 - s0*multiplier) % modulus
	if increment < 0 {
		return modulus + increment
	} else {
		return increment
	}
}

func CrackLCG(pastValues []int) *lcg.LCG {
	modulus := findModulus(pastValues)
	multiplier := findMultiplier(modulus, pastValues[0], pastValues[1], pastValues[2])
	increment := findIncrement(modulus, multiplier, pastValues[0], pastValues[1])
	seed := pastValues[len(pastValues)-1]
	clonedLCG := lcg.NewLCG(multiplier, increment, modulus, seed)
	return clonedLCG
}
