import random
import csv

def text_to_bits(text):
    return ''.join(f"{ord(c):08b}" for c in text)

def generate_streams(flag):
    bits = text_to_bits(flag)

    A, B, C, D = [], [], [], []

    for bit in bits:
        if bit == "0":
            # Must have A==B and C==D
            v1 = random.choice(["0", "1"])
            v2 = random.choice(["0", "1"])
            A.append(v1)
            B.append(v1)
            C.append(v2)
            D.append(v2)
        else:
            # Need at least one XOR=1
            if random.choice([True, False]):
                # Use A^B = 1, keep C==D
                A.append("0")
                B.append("1")
                v = random.choice(["0", "1"])
                C.append(v)
                D.append(v)
            else:
                # Use C^D = 1, keep A==B
                v = random.choice(["0", "1"])
                A.append(v)
                B.append(v)
                C.append("0")
                D.append("1")

    return A, B, C, D

def bits_to_text(bits):
    chars = [chr(int(bits[i:i+8], 2)) for i in range(0, len(bits), 8)]
    return ''.join(chars)

def simulate(A, B, C, D):
    result_bits = []
    for a, b, c, d in zip(A, B, C, D):
        ret1 = int(a) ^ int(b)
        ret2 = int(c) ^ int(d)
        result_bits.append(str(ret1 | ret2))
    return ''.join(result_bits)

if __name__ == "__main__":
    flag = "tribectf{3xpl0r1ng_th3_l0gic_lvl}"
    A, B, C, D = generate_streams(flag)

    # Save CSV
    with open("streams.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["A", "B", "C", "D"])
        for row in zip(A, B, C, D):
            writer.writerow(row)

    print("CSV written to streams.csv")

    # Verify circuit still recovers flag
    result_bits = simulate(A, B, C, D)
    recovered = bits_to_text(result_bits)
    print("Recovered flag:", recovered)
