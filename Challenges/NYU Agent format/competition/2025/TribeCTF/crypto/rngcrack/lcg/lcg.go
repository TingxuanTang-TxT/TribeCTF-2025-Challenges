package lcg

import (
	"fmt"
)

type LCG struct {
	multiplier   int
	increment    int
	modulus      int
	currentValue int
}

func NewLCG(multiplier, increment, modulus, seed int) *LCG {
	lcg := &LCG{
		multiplier:   multiplier,
		increment:    increment,
		modulus:      modulus,
		currentValue: seed,
	}
	_ = lcg.Generate()
	return lcg
}

func (lcg *LCG) Generate() int {
	oldValue := lcg.currentValue
	lcg.currentValue = (lcg.multiplier*oldValue + lcg.increment) % lcg.modulus
	return oldValue
}

func (lcg *LCG) String() string {
	return fmt.Sprintf("s_{n+1} = (%d * s_n + %d) %% %d", lcg.multiplier, lcg.increment, lcg.modulus)
}
