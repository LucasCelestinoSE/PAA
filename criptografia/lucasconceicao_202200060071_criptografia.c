/*
 * criptografia.c - Implementação conforme especificações dos slides (Bruno Prado / UFS)
 * Algoritmos numéricos: num_t, mdce, inverso_m
 * Criptografia simétrica: AES (estrutura, KeyExpansion, Cipher, Decipher, CBC)
 * Entrada: n operações (dh a b g p | d c | e m)
 * Saída: s=..., m=..., c=... em hexadecimal
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

/* ========== Definição dos dígitos (slide: estrutura número precisão dupla/simples) ========== */
typedef uint64_t d_t;
typedef uint32_t s_t;

/* Estrutura de número */
typedef struct num_t {
    s_t *d;
    uint32_t n;  /* número de limbos usados */
    uint32_t t;  /* 0 = zero, 1 = positivo */
} num_t;

/* Base numérica b = 2^32 */
const d_t b = ((d_t)(1) << (sizeof(s_t) << 3));

/* ========== Funções auxiliares num_t ========== */

static num_t *criar(void) {
    num_t *z = (num_t *)malloc(sizeof(num_t));
    if (!z) return NULL;
    z->d = NULL;
    z->n = 0;
    z->t = 0;
    return z;
}

static void destruir(num_t **z) {
    if (!z || !*z) return;
    free((*z)->d);
    (*z)->d = NULL;
    (*z)->n = 0;
    free(*z);
    *z = NULL;
}

static void zerar(num_t *z) {
    if (!z) return;
    z->t = 0;
    z->n = 0;
}

/* Atribuir: dest = src */
static void atribuir(num_t *dest, const num_t *src) {
    if (!dest || !src) return;
    if (dest == src) return;
    if (src->n == 0) {
        zerar(dest);
        return;
    }
    if (dest->n < src->n) {
        free(dest->d);
        dest->d = (s_t *)malloc((size_t)src->n * sizeof(s_t));
        if (!dest->d) return;
    }
    memcpy(dest->d, src->d, (size_t)src->n * sizeof(s_t));
    dest->n = src->n;
    dest->t = src->t;
}

/* Igual: retorna 1 se z == 1 */
static int igual(const num_t *z, int val) {
    if (!z) return 0;
    if (val == 0) return (z->t == 0 || z->n == 0);
    if (val == 1) {
        return (z->t != 0 && z->n == 1 && z->d[0] == 1);
    }
    if (z->n != 1 || z->t == 0) return 0;
    return (z->d[0] == (s_t)(val & 0xFFFFFFFFu));
}

/* Garantir capacidade de pelo menos cap limbos */
static int garantir_cap(num_t *z, uint32_t cap) {
    if (!z || cap == 0) return 0;
    if (z->d && z->n >= cap) return 1;
    s_t *novo = (s_t *)realloc(z->d, (size_t)cap * sizeof(s_t));
    if (!novo) return 0;
    z->d = novo;
    if (z->n < cap)
        memset(z->d + z->n, 0, (size_t)(cap - z->n) * sizeof(s_t));
    return 1;
}

/* Setar valor 0 ou 1 */
static void setar_um(num_t *z) {
    if (!garantir_cap(z, 1)) return;
    z->d[0] = 1;
    z->n = 1;
    z->t = 1;
}

/* Comparação: retorna <0 se a < b, 0 se a == b, >0 se a > b */
static int comparar(const num_t *a, const num_t *b) {
    if (!a || !b) return 0;
    if (a->t != b->t) return (int)a->t - (int)b->t;
    if (a->n != b->n) return (a->t ? (int)a->n - (int)b->n : (int)b->n - (int)a->n);
    for (uint32_t i = a->n; i > 0; i--) {
        uint32_t j = i - 1;
        if (a->d[j] != b->d[j]) return (a->d[j] < b->d[j]) ? -1 : 1;
    }
    return 0;
}

/* a < b */
static int menor(const num_t *a, const num_t *b) { return comparar(a, b) < 0; }
/* a <= b */
static int menor_igual(const num_t *a, const num_t *b) { return comparar(a, b) <= 0; }

/* Adição: z = x + y (z pode ser x ou y) */
static void somar(num_t *z, const num_t *x, const num_t *y) {
    if (!z || !x || !y) return;
    if (x->n == 0) { atribuir(z, y); return; }
    if (y->n == 0) { atribuir(z, x); return; }
    uint32_t nz = (x->n >= y->n ? x->n : y->n) + 1;
    if (!garantir_cap(z, nz)) return;
    d_t carry = 0;
    uint32_t i = 0;
    for (; i < x->n && i < y->n; i++) {
        d_t s = (d_t)x->d[i] + (d_t)y->d[i] + carry;
        z->d[i] = (s_t)(s & (d_t)(-1));
        carry = s >> (sizeof(s_t) * 8);
    }
    for (; i < x->n; i++) {
        d_t s = (d_t)x->d[i] + carry;
        z->d[i] = (s_t)(s & (d_t)(-1));
        carry = s >> (sizeof(s_t) * 8);
    }
    for (; i < y->n; i++) {
        d_t s = (d_t)y->d[i] + carry;
        z->d[i] = (s_t)(s & (d_t)(-1));
        carry = s >> (sizeof(s_t) * 8);
    }
    z->d[i] = (s_t)carry;
    z->n = i + (carry ? 1 : 0);
    z->t = 1;
    while (z->n > 0 && z->d[z->n - 1] == 0) z->n--;
    if (z->n == 0) z->t = 0;
}

/* Subtração: a -= b (assume a >= b) */
static void subtrair(num_t *a, const num_t *b) {
    if (!a || !b || b->n == 0) return;
    if (a->n == 0) return;
    d_t emprestado = 0;
    uint32_t i;
    for (i = 0; i < b->n; i++) {
        d_t s = (d_t)a->d[i] - (d_t)b->d[i] - emprestado;
        emprestado = (s >> (sizeof(s_t)*8)) ? 1 : 0;
        a->d[i] = (s_t)(s & (d_t)(-1));
    }
    for (; i < a->n && emprestado; i++) {
        d_t s = (d_t)a->d[i] - emprestado;
        emprestado = (s >> (sizeof(s_t)*8)) ? 1 : 0;
        a->d[i] = (s_t)(s & (d_t)(-1));
    }
    while (a->n > 0 && a->d[a->n - 1] == 0) a->n--;
    if (a->n == 0) a->t = 0;
}

/* Multiplicação: z = x * y (z pode ser igual a x ou y) */
static void multiplicar(num_t *z, const num_t *x, const num_t *y) {
    if (!z || !x || !y) return;
    if (x->n == 0 || y->n == 0) {
        zerar(z);
        return;
    }
    uint32_t nz = x->n + y->n;
    if (!garantir_cap(z, nz)) return;
    memset(z->d, 0, (size_t)nz * sizeof(s_t));
    for (uint32_t i = 0; i < x->n; i++) {
        d_t carry = 0;
        for (uint32_t j = 0; j < y->n; j++) {
            d_t p = (d_t)x->d[i] * (d_t)y->d[j] + (d_t)z->d[i + j] + carry;
            z->d[i + j] = (s_t)(p & (d_t)(-1));
            carry = p >> (sizeof(s_t) * 8);
        }
        z->d[i + y->n] = (s_t)carry;
    }
    z->n = nz;
    z->t = 1;
    while (z->n > 0 && z->d[z->n - 1] == 0) z->n--;
    if (z->n == 0) z->t = 0;
}

/* Deslocamento à esquerda: z = x * 2^bits (bits < 32) */
static void shl_bits(num_t *z, const num_t *x, unsigned bits) {
    if (!z || !x || bits == 0) { if (z && x) atribuir(z, x); return; }
    if (x->n == 0) { zerar(z); return; }
    uint32_t limbs = x->n + 1;
    if (!garantir_cap(z, limbs)) return;
    d_t carry = 0;
    for (uint32_t i = 0; i < x->n; i++) {
        d_t val = ((d_t)x->d[i] << bits) | carry;
        z->d[i] = (s_t)(val & (d_t)(-1));
        carry = val >> (sizeof(s_t) * 8);
    }
    z->d[x->n] = (s_t)carry;
    z->n = x->n + (carry ? 1 : 0);
    z->t = x->t;
}

/* Divisão: q = u / v, r = u % v (v != 0). Shift-and-subtract. */
static void dividir(num_t *q, num_t *r, const num_t *u, const num_t *v) {
    if (!q || !r || !u || !v || v->n == 0) return;
    atribuir(r, u);
    if (menor(u, v)) {
        zerar(q);
        return;
    }
    if (v->n == 1 && v->d[0] == 1) {
        atribuir(q, u);
        zerar(r);
        return;
    }
    zerar(q);
    num_t *vv = criar(), *qq = criar(), *tmp = criar(), *somaq = criar();
    if (!vv || !qq || !tmp || !somaq) {
        destruir(&vv); destruir(&qq); destruir(&tmp); destruir(&somaq);
        return;
    }
    while (menor_igual(v, r)) {
        atribuir(vv, v);
        setar_um(qq);
        /* vv = v * 2^k, qq = 2^k, com vv <= r < 2*vv */
        while (1) {
            shl_bits(tmp, vv, 1);
            if (menor(r, tmp)) break;
            atribuir(vv, tmp);
            shl_bits(tmp, qq, 1);
            atribuir(qq, tmp);
        }
        subtrair(r, vv);
        atribuir(somaq, q);
        somar(q, somaq, qq);
    }
    destruir(&vv);
    destruir(&qq);
    destruir(&tmp);
    destruir(&somaq);
}

/* Módulo: r = u mod v */
static void modulo(num_t *r, const num_t *u, const num_t *v) {
    num_t *q = criar();
    if (!q) return;
    dividir(q, r, u, v);
    destruir(&q);
}

/* ========== MDCE - Maior divisor comum estendido (slide) ========== */
/* w = gcd(u,v), w = u*x + v*y */
void mdce(num_t *w, num_t *x, num_t *y, num_t *u, num_t *v) {
    num_t *a = criar(), *b = criar();
    num_t *x1 = criar(), *x2 = criar(), *y1 = criar(), *y2 = criar();
    num_t *q = criar(), *r = criar(), *qx1 = criar(), *qy1 = criar();
    if (!a || !b || !x1 || !x2 || !y1 || !y2 || !q || !r || !qx1 || !qy1) {
        destruir(&a); destruir(&b); destruir(&x1); destruir(&x2);
        destruir(&y1); destruir(&y2); destruir(&q); destruir(&r);
        destruir(&qx1); destruir(&qy1);
        return;
    }
    atribuir(a, u);
    atribuir(b, v);
    zerar(x2); setar_um(x1);
    setar_um(y2); zerar(y1);

    while (v->n > 0) {
        dividir(q, r, u, v);
        /* qx1 = q * x1, qy1 = q * y1 */
        multiplicar(qx1, q, x1);
        multiplicar(qy1, q, y1);
        /* x = x2 - qx1, y = y2 - qy1 */
        num_t *xx = criar(), *yy = criar();
        if (xx) { atribuir(xx, x2); subtrair(xx, qx1); atribuir(x, xx); destruir(&xx); }
        if (yy) { atribuir(yy, y2); subtrair(yy, qy1); atribuir(y, yy); destruir(&yy); }
        /* u = v, v = r, x2 = x1, x1 = x, y2 = y1, y1 = y */
        atribuir(u, v);
        atribuir(v, r);
        atribuir(x2, x1);
        atribuir(x1, x);
        atribuir(y2, y1);
        atribuir(y1, y);
    }

    /* w = u, x = x2, y = y2 */
    atribuir(w, u);
    atribuir(x, x2);
    atribuir(y, y2);

    destruir(&a); destruir(&b); destruir(&x1); destruir(&x2);
    destruir(&y1); destruir(&y2); destruir(&q); destruir(&r);
    destruir(&qx1); destruir(&qy1);
}

/* ========== Inverso multiplicativo (slide) ========== */
void inverso_m(num_t *v, num_t *u, num_t *m) {
    num_t *w = criar();
    num_t *x = criar();
    num_t *y = criar();
    if (!w || !x || !y) {
        destruir(&w); destruir(&x); destruir(&y);
        return;
    }
    mdce(w, x, y, u, m);
    if (igual(w, 1))
        atribuir(v, x);
    else
        zerar(v);
    destruir(&w); destruir(&x); destruir(&y);
}

/* ========== Exponenciação modular: base^exp mod mod ========== */
static void mod_pow(num_t *resultado, const num_t *base, const num_t *exp, const num_t *mod) {
    num_t *r = criar(), *b = criar(), *e = criar();
    if (!r || !b || !e) {
        destruir(&r); destruir(&b); destruir(&e);
        return;
    }
    setar_um(r);
    atribuir(b, base);
    atribuir(e, exp);
    num_t *tmp = criar(), *q = criar(), *rem = criar();
    if (!tmp || !q || !rem) {
        destruir(&tmp); destruir(&q); destruir(&rem);
        destruir(&r); destruir(&b); destruir(&e);
        return;
    }
    while (e->n > 0 && (e->t != 0)) {
        if (e->d[0] & 1) {
            multiplicar(tmp, r, b);
            modulo(r, tmp, mod);
        }
        multiplicar(tmp, b, b);
        modulo(b, tmp, mod);
        /* e = e / 2 */
        d_t carry = 0;
        for (uint32_t i = e->n; i > 0; i--) {
            uint32_t j = i - 1;
            d_t val = (d_t)e->d[j] + (carry << (sizeof(s_t)*8));
            e->d[j] = (s_t)(val >> 1);
            carry = val & 1;
        }
        while (e->n > 0 && e->d[e->n - 1] == 0) e->n--;
        if (e->n == 0) e->t = 0;
    }
    atribuir(resultado, r);
    destruir(&r); destruir(&b); destruir(&e);
    destruir(&tmp); destruir(&q); destruir(&rem);
}

/* ========== Conversão hex <-> num_t ========== */
static int hex_char_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static void num_from_hex(num_t *z, const char *hex) {
    zerar(z);
    if (!hex) return;
    while (*hex && (*hex == ' ' || *hex == '\t')) hex++;
    size_t len = 0;
    const char *p = hex;
    while (hex_char_val(*p) >= 0) { len++; p++; }
    if (len == 0) return;
    /* Cada 8 hex chars = 1 limb (32 bits). Processar do menos significativo ao mais. */
    uint32_t limbs = (uint32_t)((len + 7) / 8);
    if (!garantir_cap(z, limbs)) return;
    z->n = limbs;
    z->t = 1;
    memset(z->d, 0, (size_t)limbs * sizeof(s_t));
    for (size_t i = 0; i < len; i++) {
        int v = hex_char_val(hex[len - 1 - i]);
        if (v < 0) break;
        uint32_t limb_idx = (uint32_t)(i / 8);
        uint32_t shift = (uint32_t)(i % 8) * 4;
        z->d[limb_idx] |= (s_t)((unsigned)v << shift);
    }
    while (z->n > 0 && z->d[z->n - 1] == 0) z->n--;
    if (z->n == 0) z->t = 0;
}

static void num_to_hex(const num_t *z, char *buf, size_t buf_size) {
    if (!z || !buf || buf_size == 0) { if (buf_size) buf[0] = '\0'; return; }
    if (z->n == 0 || z->t == 0) {
        if (buf_size >= 2) { buf[0] = '0'; buf[1] = '\0'; }
        else if (buf_size) buf[0] = '\0';
        return;
    }
    static const char hex_digits[] = "0123456789ABCDEF";
    size_t pos = 0;
    int leading = 1;
    for (uint32_t j = z->n; j > 0; j--) {
        uint32_t i = j - 1;
        for (int s = 28; s >= 0; s -= 4) {
            unsigned nibble = (z->d[i] >> s) & 0xF;
            if (leading && nibble == 0 && (i > 0 || s > 0)) continue;
            leading = 0;
            if (pos + 2 <= buf_size) {
                buf[pos++] = hex_digits[nibble];
            }
        }
    }
    if (pos == 0 && buf_size) buf[pos++] = '0';
    if (pos < buf_size) buf[pos] = '\0';
    else if (buf_size) buf[buf_size - 1] = '\0';
}

/* ========== AES (slides Criptografia simétrica) ========== */

typedef struct aes_t {
    uint8_t *c0;
    uint8_t *k;
    uint8_t *ke;
    size_t Nk;
} aes_t;

/* Round Constant (1 <= i <= 10) */
const uint8_t rcon[11] = { 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36 };

/* AES S-box (FIPS-197) */
static const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
    0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
    0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,
    0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
    0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
    0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
    0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
    0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,
    0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
    0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,
    0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,
    0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,
    0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,
    0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static const uint8_t inv_sbox[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38,
    0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87,
    0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d,
    0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2,
    0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16,
    0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda,
    0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a,
    0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02,
    0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea,
    0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85,
    0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89,
    0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20,
    0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31,
    0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d,
    0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0,
    0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26,
    0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

/* [d1,d2,d3,d0] <- [d0,d1,d2,d3] */
static void RotWord(uint8_t *data) {
    uint8_t rotation[4] = { data[1], data[2], data[3], data[0] };
    memcpy(data, rotation, 4);
}

/* [sbox[d0], ...] <- [d0,d1,d2,d3] */
static void SubWord(uint8_t *data) {
    data[0] = sbox[data[0]];
    data[1] = sbox[data[1]];
    data[2] = sbox[data[2]];
    data[3] = sbox[data[3]];
}

static void WriteWord(uint8_t *out, size_t out_off, const uint8_t *in, size_t in_off) {
    memcpy(out + out_off, in + in_off, 4);
}

static void WriteWordXor(uint8_t *out, size_t out_off, const uint8_t *a, size_t a_off, const uint8_t *b) {
    for (int i = 0; i < 4; i++)
        out[out_off + i] = a[a_off + i] ^ b[i];
}

/* KeyExpansion: out = expanded key, in = key, Nk = key words (4, 6 ou 8) */
static void KeyExpansion(uint8_t *out, const uint8_t *in, uint8_t Nk) {
    const uint8_t Nr = Nk + 6;
    uint8_t temp[4];
    /* Primeira rodada é a própria chave */
    for (uint8_t i = 0; i < Nk; i++)
        WriteWord(out, (size_t)(i << 2), in, (size_t)(i << 2));
    /* Gerando as rodadas a partir das anteriores */
    for (uint8_t i = Nk; i < (uint8_t)((Nr + 1) << 2); i++) {
        WriteWord(temp, 0, out, (size_t)((i - 1) << 2));
        if (i % Nk == 0) {
            RotWord(temp);
            SubWord(temp);
            temp[0] = temp[0] ^ rcon[i / Nk];
        } else if (Nk > 6 && i % Nk == 4) {
            SubWord(temp);
        }
        WriteWordXor(out, (size_t)(i << 2), out, (size_t)((i - Nk) << 2), temp);
    }
}

/* State: state[col][row] = byte (column-major como no FIPS) */
static void ReadInput(uint8_t state[4][4], const uint8_t *m) {
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            state[r][c] = m[c * 4 + r];
}

static void WriteOutput(uint8_t *out, const uint8_t state[4][4]) {
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            out[c * 4 + r] = state[r][c];
}

static void AddRoundKey(uint8_t state[4][4], const uint8_t *k, uint8_t round) {
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            state[r][c] ^= k[round * 16 + c * 4 + r];
}

static void SubBytes(uint8_t state[4][4]) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            state[i][j] = sbox[state[i][j]];
}

static void ShiftRows(uint8_t state[4][4]) {
    uint8_t t;
    t = state[1][0]; state[1][0] = state[1][1]; state[1][1] = state[1][2]; state[1][2] = state[1][3]; state[1][3] = t;
    t = state[2][0]; state[2][0] = state[2][2]; state[2][2] = t;
    t = state[2][1]; state[2][1] = state[2][3]; state[2][3] = t;
    t = state[3][0]; state[3][0] = state[3][3]; state[3][3] = state[3][2]; state[3][2] = state[3][1]; state[3][1] = t;
}

static uint8_t gmul(uint8_t a, uint8_t b) {
    uint8_t p = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 1) p ^= a;
        if (a & 0x80) a = (a << 1) ^ 0x1b;
        else a <<= 1;
        b >>= 1;
    }
    return p;
}

static void MixColumns(uint8_t state[4][4]) {
    for (int c = 0; c < 4; c++) {
        uint8_t a0 = state[0][c], a1 = state[1][c], a2 = state[2][c], a3 = state[3][c];
        state[0][c] = gmul(a0, 2) ^ gmul(a1, 3) ^ a2 ^ a3;
        state[1][c] = a0 ^ gmul(a1, 2) ^ gmul(a2, 3) ^ a3;
        state[2][c] = a0 ^ a1 ^ gmul(a2, 2) ^ gmul(a3, 3);
        state[3][c] = gmul(a0, 3) ^ a1 ^ a2 ^ gmul(a3, 2);
    }
}

/* Procedimento de encriptação (slide) */
static void Cipher(uint8_t *c, const uint8_t *m, const uint8_t *k, uint8_t Nr) {
    uint8_t state[4][4];
    ReadInput(state, m);
    AddRoundKey(state, k, 0);
    for (uint8_t i = 1; i < Nr; i++) {
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(state, k, i);
    }
    SubBytes(state);
    ShiftRows(state);
    AddRoundKey(state, k, Nr);
    WriteOutput(c, state);
}

static void InvShiftRows(uint8_t state[4][4]) {
    uint8_t t;
    t = state[1][3]; state[1][3] = state[1][2]; state[1][2] = state[1][1]; state[1][1] = state[1][0]; state[1][0] = t;
    t = state[2][0]; state[2][0] = state[2][2]; state[2][2] = t;
    t = state[2][1]; state[2][1] = state[2][3]; state[2][3] = t;
    t = state[3][0]; state[3][0] = state[3][1]; state[3][1] = state[3][2]; state[3][2] = state[3][3]; state[3][3] = t;
}

static void InvSubBytes(uint8_t state[4][4]) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            state[i][j] = inv_sbox[state[i][j]];
}

static void InvMixColumns(uint8_t state[4][4]) {
    for (int c = 0; c < 4; c++) {
        uint8_t a0 = state[0][c], a1 = state[1][c], a2 = state[2][c], a3 = state[3][c];
        state[0][c] = gmul(a0, 0x0e) ^ gmul(a1, 0x0b) ^ gmul(a2, 0x0d) ^ gmul(a3, 0x09);
        state[1][c] = gmul(a0, 0x09) ^ gmul(a1, 0x0e) ^ gmul(a2, 0x0b) ^ gmul(a3, 0x0d);
        state[2][c] = gmul(a0, 0x0d) ^ gmul(a1, 0x09) ^ gmul(a2, 0x0e) ^ gmul(a3, 0x0b);
        state[3][c] = gmul(a0, 0x0b) ^ gmul(a1, 0x0d) ^ gmul(a2, 0x09) ^ gmul(a3, 0x0e);
    }
}

static void Decipher(uint8_t *m, const uint8_t *c, const uint8_t *k, uint8_t Nr) {
    uint8_t state[4][4];
    ReadInput(state, c);
    AddRoundKey(state, k, Nr);
    for (uint8_t i = Nr - 1; i >= 1; i--) {
        InvShiftRows(state);
        InvSubBytes(state);
        AddRoundKey(state, k, i);
        InvMixColumns(state);
    }
    InvShiftRows(state);
    InvSubBytes(state);
    AddRoundKey(state, k, 0);
    WriteOutput(m, state);
}

static void Xor(uint8_t *dst, const uint8_t *src1, const uint8_t *src2) {
    for (int i = 0; i < 16; i++)
        dst[i] = src1[i] ^ src2[i];
}

/* Procedimento de decriptação AES-CBC (slide) */
static void aes_d_cbc(uint8_t *m, const uint8_t *c, size_t l, aes_t *aes) {
    const uint8_t Nr = (uint8_t)(aes->Nk + 6);
    const uint8_t *ci1 = aes->c0;
    for (size_t i = 0; i < l; i += 16) {
        Decipher(m + i, c + i, aes->ke, Nr);
        Xor(m + i, m + i, ci1);
        ci1 = c + i;
    }
    memcpy(aes->c0, ci1, 16);
}

/* Procedimento de encriptação AES-CBC (espelho do aes_d_cbc) */
static void aes_e_cbc(uint8_t *c, const uint8_t *m, size_t l, aes_t *aes) {
    const uint8_t Nr = (uint8_t)(aes->Nk + 6);
    uint8_t *ci1 = aes->c0;
    for (size_t i = 0; i < l; i += 16) {
        Xor(c + i, m + i, ci1);
        Cipher(c + i, c + i, aes->ke, Nr);
        ci1 = (uint8_t *)(c + i);
    }
    memcpy(aes->c0, ci1, 16);
}

static void AddCounter(uint8_t *ti) {
    for (int i = 15; i >= 0; i--) {
        if (++ti[i] != 0) break;
    }
}

/* AES-CTR (slide) - disponível para uso em modo CTR */
static void __attribute__((unused)) aes_x_ctr(uint8_t *out, const uint8_t *in, size_t l, aes_t *aes) {
    const uint8_t Nr = (uint8_t)(aes->Nk + 6);
    uint8_t *ti = aes->c0;
    for (size_t i = 0; i < l; i += 16) {
        Cipher(out + i, ti, aes->ke, Nr);
        Xor(out + i, out + i, in + i);
        AddCounter(ti);
    }
}

/* ========== Parsing e main ========== */

static size_t hex_to_bytes(const char *hex, uint8_t *out, size_t max_len) {
    size_t len = 0;
    while (hex[len] && hex_char_val(hex[len]) >= 0) len++;
    len /= 2;
    if (len > max_len) len = max_len;
    for (size_t i = 0; i < len; i++) {
        int h = hex_char_val(hex[i*2]), l = hex_char_val(hex[i*2+1]);
        if (h < 0 || l < 0) break;
        out[i] = (uint8_t)((h << 4) | l);
    }
    return len;
}

static void bytes_to_hex(const uint8_t *buf, size_t len, char *out, size_t out_size) {
    static const char hex[] = "0123456789ABCDEF";
    size_t i = 0;
    for (; i < len && i * 2 + 2 <= out_size; i++) {
        out[i*2]   = hex[buf[i] >> 4];
        out[i*2+1] = hex[buf[i] & 0xF];
    }
    if (i * 2 < out_size) out[i*2] = '\0';
    else if (out_size) out[out_size-1] = '\0';
}

#define MAX_HEX_LEN  4096
#define MAX_BYTES    (MAX_HEX_LEN/2)
#define KEY_BYTES    16
#define IV_BYTES     16
#define AES128_NK    4
#define AES_KEY_EXPANDED_BYTES  (16 * 11)  /* 11 round keys for AES-128 */

static aes_t aes_ctx;

#define DEFAULT_INPUT   "criptografia.input"
#define DEFAULT_OUTPUT "criptografia.output"

int main(int argc, char **argv) {
    const char *input_file = (argc > 1) ? argv[1] : DEFAULT_INPUT;
    const char *output_file = (argc > 2) ? argv[2] : DEFAULT_OUTPUT;

    FILE *fin = fopen(input_file, "r");
    if (!fin) {
        fprintf(stderr, "Erro: nao foi possivel abrir entrada '%s'\n", input_file);
        return 1;
    }
    FILE *fout = fopen(output_file, "w");
    if (!fout) {
        fprintf(stderr, "Erro: nao foi possivel abrir saida '%s'\n", output_file);
        fclose(fin);
        return 1;
    }

    int n;
    if (fscanf(fin, "%d", &n) != 1) {
        fclose(fin);
        fclose(fout);
        return 1;
    }
    while (fgetc(fin) != '\n' && !feof(fin))
        ;

    /* Alocação fixa para AES: chave 16 bytes, IV 16 bytes, expanded key */
    uint8_t key[KEY_BYTES];
    uint8_t iv[IV_BYTES];
    uint8_t ke[AES_KEY_EXPANDED_BYTES];
    memset(key, 0, KEY_BYTES);
    memset(iv, 0, IV_BYTES);
    aes_ctx.k = key;
    aes_ctx.ke = ke;
    aes_ctx.c0 = iv;
    aes_ctx.Nk = AES128_NK;

    for (int op = 0; op < n; op++) {
        char line[8192];
        if (!fgets(line, sizeof(line), fin)) break;
        char *cmd = line;
        while (*cmd == ' ' || *cmd == '\t') cmd++;
        if (cmd[0] == 'd' && (cmd[1] == 'h' || cmd[1] == ' ')) {
            if (cmd[1] == 'h') {
                /* dh a b g p */
                cmd += 2;
                while (*cmd == ' ') cmd++;
                char *a_str = cmd;
                while (*cmd && *cmd != ' ') cmd++;
                if (*cmd) *cmd++ = '\0';
                while (*cmd == ' ') cmd++;
                char *b_str = cmd;
                while (*cmd && *cmd != ' ') cmd++;
                if (*cmd) *cmd++ = '\0';
                while (*cmd == ' ') cmd++;
                char *g_str = cmd;
                while (*cmd && *cmd != ' ') cmd++;
                if (*cmd) *cmd++ = '\0';
                while (*cmd == ' ') cmd++;
                char *p_str = cmd;
                while (*cmd && *cmd != '\n' && *cmd != '\0') cmd++;
                if (*cmd) *cmd = '\0';
                while (*p_str == ' ') p_str++;

                num_t *a = criar(), *b = criar(), *g = criar(), *p = criar();
                num_t *s = criar();
                if (!a || !b || !g || !p || !s) {
                    destruir(&a); destruir(&b); destruir(&g); destruir(&p); destruir(&s);
                    continue;
                }
                num_from_hex(a, a_str);
                num_from_hex(b, b_str);
                num_from_hex(g, g_str);
                num_from_hex(p, p_str);

                /* s = g^(a*b) mod p (shared secret) */
                num_t *ab = criar(), *exp = criar();
                if (ab && exp) {
                    multiplicar(ab, a, b);
                    mod_pow(s, g, ab, p);
                    destruir(&ab);
                    destruir(&exp);
                }

                /* Saída s= em hex (128 bits = 32 hex chars, conforme formato do exercício) */
                char hex_buf[MAX_HEX_LEN];
                num_to_hex(s, hex_buf, sizeof(hex_buf));
                size_t hex_len = strlen(hex_buf);
                const char *s_display = hex_buf;
                if (hex_len > 32)
                    s_display = hex_buf + hex_len - 32;
                fprintf(fout, "s=%s\n", s_display);

                /* Chave AES = 128 bits baixos do segredo (últimos 32 hex = valor em s=) */
                memset(key, 0, KEY_BYTES);
                memset(iv, 0, IV_BYTES);
                if (hex_len >= 32) {
                    char key_hex[33];
                    memcpy(key_hex, s_display, 32);
                    key_hex[32] = '\0';
                    hex_to_bytes(key_hex, key, KEY_BYTES);
                } else {
                    hex_to_bytes(hex_buf, key, KEY_BYTES);
                }

                KeyExpansion(ke, key, (uint8_t)AES128_NK);

                destruir(&a); destruir(&b); destruir(&g); destruir(&p); destruir(&s);
            } else {
                /* d c - decriptar */
                cmd += 1;
                while (*cmd == ' ') cmd++;
                char *c_hex = cmd;
                while (*c_hex != '\n' && *c_hex) c_hex++;
                *c_hex = '\0';

                uint8_t cip[MAX_BYTES];
                size_t clen = hex_to_bytes(cmd, cip, sizeof(cip));
                if (clen % 16 != 0) {
                    size_t pad = 16 - (clen % 16);
                    memset(cip + clen, 0, pad);
                    clen += pad;
                }
                uint8_t msg[MAX_BYTES];
                aes_d_cbc(msg, cip, clen, &aes_ctx);
                char out_hex[MAX_HEX_LEN];
                bytes_to_hex(msg, clen, out_hex, sizeof(out_hex));
                fprintf(fout, "m=%s\n", out_hex);
            }
        } else if (cmd[0] == 'e' && cmd[1] == ' ') {
            /* e m - encriptar (IV zerado para reprodutibilidade, como no gabarito) */
            memset(aes_ctx.c0, 0, 16);
            cmd += 2;
            while (*cmd == ' ') cmd++;
            char *m_hex = cmd;
            while (*m_hex != '\n' && *m_hex) m_hex++;
            *m_hex = '\0';

            /* Garantir número par de hex: se ímpar, preencher com '0' à esquerda */
            size_t hex_len = 0;
            while (cmd[hex_len] && hex_char_val(cmd[hex_len]) >= 0) hex_len++;
            char hex_padded[MAX_HEX_LEN];
            if (hex_len > 0 && (hex_len % 2) != 0) {
                hex_padded[0] = '0';
                memcpy(hex_padded + 1, cmd, hex_len + 1);
                cmd = hex_padded;
            }

            uint8_t msg[MAX_BYTES];
            size_t mlen_orig = hex_to_bytes(cmd, msg, sizeof(msg));
            size_t mlen = mlen_orig;
            /* Padding para AES: zeros no final até múltiplo de 16 */
            if (mlen % 16 != 0) {
                size_t pad = 16 - (mlen % 16);
                memset(msg + mlen, 0, pad);
                mlen += pad;
            }
            uint8_t cip[MAX_BYTES];
            aes_e_cbc(cip, msg, mlen, &aes_ctx);
            /* Saída: apenas os primeiros mlen_orig bytes do ciphertext (como no gabarito) */
            char out_hex[MAX_HEX_LEN];
            bytes_to_hex(cip, mlen_orig, out_hex, sizeof(out_hex));
            fprintf(fout, "c=%s\n", out_hex);
        }
    }
    fclose(fin);
    fclose(fout);
    return 0;
}
