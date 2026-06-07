#include "crypto/bn.h"


/* Functions for shifting number in-place. */
static void _lshift_one_bit(struct bn* a);
static void _rshift_one_bit(struct bn* a);
static void _lshift_word(struct bn* a, int nwords);
static void _rshift_word(struct bn* a, int nwords);

/* Helper function to convert a single hex character to its integer value */
static int _hex_char_to_int(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

/* Helper function to convert a nibble (4 bits) to a hex character */
static char _nibble_to_hex_char(uint8_t nibble)
{
    nibble &= 0x0F;
    if (nibble < 10) {
        return '0' + nibble;
    } else {
        return 'a' + (nibble - 10);
    }
}


/* Public / Exported functions. */
void bignum_init(struct bn* n)
{
    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        n->array[i] = 0;
    }
}


void bignum_from_int(struct bn* n, DTYPE_TMP i)
{
    bignum_init(n);

    /* Endianness issue if machine is not little-endian? */
#ifdef WORD_SIZE
#if (WORD_SIZE == 1)
    n->array[0] = (i & 0x000000ff);
    n->array[1] = (i & 0x0000ff00) >> 8;
    n->array[2] = (i & 0x00ff0000) >> 16;
    n->array[3] = (i & 0xff000000) >> 24;
#elif (WORD_SIZE == 2)
    n->array[0] = (i & 0x0000ffff);
    n->array[1] = (i & 0xffff0000) >> 16;
#elif (WORD_SIZE == 4)
    n->array[0] = i;
    DTYPE_TMP num_32 = 32;
    DTYPE_TMP tmp = i >> num_32; /* bit-shift with U64 operands to force 64-bit results */
    n->array[1] = tmp;
#endif
#endif
}


int bignum_to_int(struct bn* n)
{
    int ret = 0;

    /* Endianness issue if machine is not little-endian? */
#if (WORD_SIZE == 1)
    ret += n->array[0];
    ret += n->array[1] << 8;
    ret += n->array[2] << 16;
    ret += n->array[3] << 24;  
#elif (WORD_SIZE == 2)
    ret += n->array[0];
    ret += n->array[1] << 16;
#elif (WORD_SIZE == 4)
    ret += n->array[0];
#endif

    return ret;
}


void bignum_from_string(struct bn* n, char* str, int nbytes)
{
    bignum_init(n);

    DTYPE tmp;
    int i = nbytes - 1; // Start from the last character of the string
    int j = 0;          // Index into the bignum array
    int k = 0;          // Bit counter for the current DTYPE word

    // Process string from right to left (LSB to MSB)
    while (i >= 0 && j < BN_ARRAY_SIZE)
    {
        tmp = _hex_char_to_int(str[i]);
        n->array[j] |= (tmp << k);
        
        k += 4; // Each hex char is 4 bits
        if (k >= (WORD_SIZE * 8))
        {
            k = 0;
            j++;
        }
        i--;
    }
}


void bignum_to_string(struct bn* n, char* str, int nbytes)
{
    int j = BN_ARRAY_SIZE - 1; // Start from the most significant word
    int i = 0;                 // Index into the output string
    uint8_t leading_zeros = true;

    // Iterate through the bignum array from MSB to LSB
    while (j >= 0)
    {
        // Iterate through the nibbles in the current word
        for (int k = (WORD_SIZE * 8) - 4; k >= 0; k -= 4)
        {
            uint8_t nibble = (n->array[j] >> k) & 0x0F;

            if (nibble == 0 && leading_zeros && !(j == 0 && k == 0))
            {
                // Skip leading zeros, but always print at least one character
                continue;
            }
            
            if (i < (nbytes - 1))
            {
                str[i++] = _nibble_to_hex_char(nibble);
                leading_zeros = false;
            }
        }
        j--;
    }
    str[i] = '\0'; // Null-terminate the string
}

void bignum_from_bytes(struct bn* n, const uint8_t* bytes, int len) {
    bignum_init(n);

    int word_idx = 0;
    int byte_in_word = 0;

    // Process the input bytes from right to left (LSB to MSB)
    for (int i = len - 1; i >= 0; i--)
    {
        // Prevent writing past the end of the bignum array
        if (word_idx >= BN_ARRAY_SIZE) {
            break;
        }

        // Place the byte into the correct position in the current word
        DTYPE byte_val = (DTYPE)bytes[i];
        n->array[word_idx] |= (byte_val << (byte_in_word * 8));

        byte_in_word++;
        if (byte_in_word >= WORD_SIZE)
        {
            byte_in_word = 0;
            word_idx++;
        }
    }
}


void bignum_to_bytes(struct bn* n, uint8_t* bytes, int len)
{
    int word_idx = 0;
    int byte_in_word = 0;

    // Write to the output bytes from right to left (LSB to MSB)
    for (int i = len - 1; i >= 0; i--)
    {
        if (word_idx >= BN_ARRAY_SIZE) {
            bytes[i] = 0; // Zero-pad if bignum is smaller than buffer
            continue;
        }

        // Extract the correct byte from the current word
        bytes[i] = (n->array[word_idx] >> (byte_in_word * 8)) & 0xFF;

        byte_in_word++;
        if (byte_in_word >= WORD_SIZE)
        {
            byte_in_word = 0;
            word_idx++;
        }
    }
}



void bignum_dec(struct bn* n)
{
    DTYPE tmp;
    DTYPE res;
    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        tmp = n->array[i];
        res = tmp - 1;
        n->array[i] = res;

        if (!(res > tmp))
        {
            break;
        }
    }
}


void bignum_inc(struct bn* n)
{
    DTYPE res;
    DTYPE_TMP tmp;
    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        tmp = n->array[i];
        res = tmp + 1;
        n->array[i] = res;

        if (res > tmp)
        {
            break;
        }
    }
}


void bignum_add(struct bn* a, struct bn* b, struct bn* c)
{
    DTYPE_TMP tmp;
    int carry = 0;
    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        tmp = (DTYPE_TMP)a->array[i] + b->array[i] + carry;
        carry = (tmp > MAX_VAL);
        c->array[i] = (tmp & MAX_VAL);
    }
}


void bignum_sub(struct bn* a, struct bn* b, struct bn* c)
{
    DTYPE_TMP res;
    DTYPE_TMP tmp1;
    DTYPE_TMP tmp2;
    int borrow = 0;
    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        tmp1 = (DTYPE_TMP)a->array[i] + (MAX_VAL + 1);
        tmp2 = (DTYPE_TMP)b->array[i] + borrow;
        res = (tmp1 - tmp2);
        c->array[i] = (DTYPE)(res & MAX_VAL);
        borrow = (res <= MAX_VAL);
    }
}


void bignum_mul(struct bn* a, struct bn* b, struct bn* c)
{
    struct bn row;
    struct bn tmp;
    int i, j;

    bignum_init(c);

    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        bignum_init(&row);

        for (j = 0; j < BN_ARRAY_SIZE; ++j)
        {
            if (i + j < BN_ARRAY_SIZE)
            {
                bignum_init(&tmp);
                DTYPE_TMP intermediate = ((DTYPE_TMP)a->array[i] * (DTYPE_TMP)b->array[j]);
                bignum_from_int(&tmp, intermediate);
                _lshift_word(&tmp, i + j);
                bignum_add(&tmp, &row, &row);
            }
        }
        bignum_add(c, &row, c);
    }
}


void bignum_div(struct bn* a, struct bn* b, struct bn* c)
{
    struct bn current;
    struct bn denom;
    struct bn tmp;

    bignum_from_int(&current, 1);
    bignum_assign(&denom, b);
    bignum_assign(&tmp, a);

    const DTYPE_TMP half_max = 1 + (DTYPE_TMP)(MAX_VAL / 2);
    uint8_t overflow = false;
    while (bignum_cmp(&denom, a) != LARGER)
    {
        if (denom.array[BN_ARRAY_SIZE - 1] >= half_max)
        {
            overflow = true;
            break;
        }
        _lshift_one_bit(&current);
        _lshift_one_bit(&denom);
    }
    if (!overflow)
    {
        _rshift_one_bit(&denom);
        _rshift_one_bit(&current);
    }
    bignum_init(c);

    while (!bignum_is_zero(&current))
    {
        if (bignum_cmp(&tmp, &denom) != SMALLER)
        {
            bignum_sub(&tmp, &denom, &tmp);
            bignum_or(c, &current, c);
        }
        _rshift_one_bit(&current);
        _rshift_one_bit(&denom);
    }
}


void bignum_lshift(struct bn* a, struct bn* b, int nbits)
{
    bignum_assign(b, a);
    const int nbits_pr_word = (WORD_SIZE * 8);
    int nwords = nbits / nbits_pr_word;
    if (nwords != 0)
    {
        _lshift_word(b, nwords);
        nbits -= (nwords * nbits_pr_word);
    }

    if (nbits != 0)
    {
        int i;
        for (i = (BN_ARRAY_SIZE - 1); i > 0; --i)
        {
            b->array[i] = (b->array[i] << nbits) | (b->array[i - 1] >> ((8 * WORD_SIZE) - nbits));
        }
        b->array[i] <<= nbits;
    }
}


void bignum_rshift(struct bn* a, struct bn* b, int nbits)
{
    bignum_assign(b, a);
    const int nbits_pr_word = (WORD_SIZE * 8);
    int nwords = nbits / nbits_pr_word;
    if (nwords != 0)
    {
        _rshift_word(b, nwords);
        nbits -= (nwords * nbits_pr_word);
    }

    if (nbits != 0)
    {
        int i;
        for (i = 0; i < (BN_ARRAY_SIZE - 1); ++i)
        {
            b->array[i] = (b->array[i] >> nbits) | (b->array[i + 1] << ((8 * WORD_SIZE) - nbits));
        }
        b->array[i] >>= nbits;
    }
}


void bignum_mod(struct bn* a, struct bn* b, struct bn* c)
{
    struct bn tmp;
    bignum_divmod(a, b, &tmp, c);
}

void bignum_divmod(struct bn* a, struct bn* b, struct bn* c, struct bn* d)
{
    struct bn tmp;
    bignum_div(a, b, c);
    bignum_mul(c, b, &tmp);
    bignum_sub(a, &tmp, d);
}


void bignum_and(struct bn* a, struct bn* b, struct bn* c)
{
    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        c->array[i] = (a->array[i] & b->array[i]);
    }
}


void bignum_or(struct bn* a, struct bn* b, struct bn* c)
{
    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        c->array[i] = (a->array[i] | b->array[i]);
    }
}


void bignum_xor(struct bn* a, struct bn* b, struct bn* c)
{
    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        c->array[i] = (a->array[i] ^ b->array[i]);
    }
}


int bignum_cmp(struct bn* a, struct bn* b)
{
    int i = BN_ARRAY_SIZE;
    do
    {
        i -= 1;
        if (a->array[i] > b->array[i])
        {
            return LARGER;
        }
        else if (a->array[i] < b->array[i])
        {
            return SMALLER;
        }
    }
    while (i != 0);

    return EQUAL;
}


int bignum_is_zero(struct bn* n)
{
    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        if (n->array[i])
        {
            return 0;
        }
    }
    return 1;
}


void bignum_pow(struct bn* a, struct bn* b, struct bn* c)
{
    struct bn tmp;
    bignum_init(c);

    if (bignum_is_zero(b))
    {
        bignum_inc(c);
    }
    else
    {
        struct bn bcopy;
        bignum_assign(&bcopy, b);
        bignum_assign(&tmp, a);
        bignum_dec(&bcopy);
    
        while (!bignum_is_zero(&bcopy))
        {
            bignum_mul(&tmp, a, c);
            bignum_dec(&bcopy);
            bignum_assign(&tmp, c);
        }
        bignum_assign(c, &tmp);
    }
}

void bignum_isqrt(struct bn *a, struct bn* b)
{
    struct bn low, high, mid, tmp;

    bignum_init(&low);
    bignum_assign(&high, a);
    bignum_rshift(&high, &mid, 1);
    bignum_inc(&mid);

    while (bignum_cmp(&high, &low) > 0)  
    {
        bignum_mul(&mid, &mid, &tmp);
        if (bignum_cmp(&tmp, a) > 0) 
        {
            bignum_assign(&high, &mid);
            bignum_dec(&high);
        }
        else 
        {
            bignum_assign(&low, &mid);
        }
        bignum_sub(&high, &low, &mid);
        _rshift_one_bit(&mid);
        bignum_add(&low, &mid, &mid);
        bignum_inc(&mid);
    }
    bignum_assign(b, &low);
}


void bignum_assign(struct bn* dst, struct bn* src)
{
    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        dst->array[i] = src->array[i];
    }
}


/* Private / Static functions. */
static void _rshift_word(struct bn* a, int nwords)
{
    int i;
    if (nwords >= BN_ARRAY_SIZE)
    {
        bignum_init(a);
        return;
    }

    for (i = 0; i < BN_ARRAY_SIZE - nwords; ++i)
    {
        a->array[i] = a->array[i + nwords];
    }
    for (; i < BN_ARRAY_SIZE; ++i)
    {
        a->array[i] = 0;
    }
}


static void _lshift_word(struct bn* a, int nwords)
{
    int i;
    for (i = (BN_ARRAY_SIZE - 1); i >= nwords; --i)
    {
        a->array[i] = a->array[i - nwords];
    }
    for (; i >= 0; --i)
    {
        a->array[i] = 0;
    }  
}


static void _lshift_one_bit(struct bn* a)
{
    int i;
    for (i = (BN_ARRAY_SIZE - 1); i > 0; --i)
    {
        a->array[i] = (a->array[i] << 1) | (a->array[i - 1] >> ((8 * WORD_SIZE) - 1));
    }
    a->array[0] <<= 1;
}


static void _rshift_one_bit(struct bn* a)
{
    int i;
    for (i = 0; i < (BN_ARRAY_SIZE - 1); ++i)
    {
        a->array[i] = (a->array[i] >> 1) | (a->array[i + 1] << ((8 * WORD_SIZE) - 1));
    }
    a->array[BN_ARRAY_SIZE - 1] >>= 1;
}
