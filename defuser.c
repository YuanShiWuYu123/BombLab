#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    if (!cap || !count) return 1;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {

    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t middle = (uint32_t)(challenge >> 16);
    uint32_t step0 = (high + 0x5926u) ^ low;
    uint32_t step1 = 3u * (high - middle) + 0xbc40u;
    uint32_t value = ((step1 + 4u * low) ^ 0xada4u) ^ step0;
    return write_answer(out, cap, &value, 1);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[6];
    values[0] = (uint32_t)((challenge >> 22) & 31u) ^ 0x17u;

    for (unsigned i = 1; i < 6; ++i) {
        uint32_t digit = (uint32_t)((challenge >> (15u + 4u * (i - 1u))) & 15u);
        values[i] = 3u * values[i - 1] + (digit ^ (10u + i));
    }
    return write_answer(out, cap, values, 6);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[2];
    uint32_t x = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);
    values[0] = (uint32_t)challenge & 7u;

    switch (values[0]) {
        case 0: values[1] = 5u * x + 0x148au; break;
        case 1: values[1] = 3u * x + 0x574du; break;
        case 2: values[1] = 5u * x + 0xbce3u; break;
        case 3: values[1] = (4u * x) ^ 0xf7a2u; break;
        case 4: values[1] = (x ^ 0x3775u) + (x >> 2); break;
        case 5: values[1] = (4u * x) ^ 0x30f5u; break;
        case 6: values[1] = 5u * x + 0x12f70u; break;
        default: values[1] = (x ^ 0x4eau) + (x >> 2); break;
    }
    return write_answer(out, cap, values, 2);
}

static uint32_t walk(uint32_t target, uint32_t low, uint32_t high, uint32_t key) {
    uint32_t mid = low + ((high - low) >> 1);
    if (target == mid) return (mid + 0x93u) ^ key;
    if (target < mid)
        return (5u * walk(target, low, mid - 1u, key)) ^ (mid + key) ^ 0xcu;
    return (5u * walk(target, mid + 1u, high, key)) ^ (mid + key) ^ 8u;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t key = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    uint32_t value = walk((uint32_t)challenge & 127u, 0, 127, key);
    return write_answer(out, cap, &value, 1);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {

    static const struct { uint32_t value, id; int next; } nodes[12] = {
        {0xe96du, 0x655u, 8}, {0xef69u, 0x564u, 10},
        {0x6ba7u, 0x40fu, 1}, {0xea01u, 0x227u, 0},
        {0x8a19u, 0x04fu, 3}, {0xd436u, 0x371u, -1},
        {0x4810u, 0x960u, 2}, {0x0a79u, 0x76cu, 9},
        {0x8cf8u, 0xb7au, 7}, {0x4de1u, 0xa58u, 5},
        {0x6918u, 0x873u, 11}, {0x7066u, 0x152u, 4}
    };
    static const unsigned heads[8] = {6, 2, 1, 10, 11, 4, 3, 0};
    unsigned index = heads[challenge & 7u];
    unsigned hops = (unsigned)((challenge >> 3) & 3u);
    uint32_t values[2] = {
        (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32) ^ 0x6da3u, 0
    };
    for (unsigned step = 0; step <= hops; ++step) {
        values[0] = (values[0] ^ nodes[index].value) + nodes[index].id;
        values[1] = nodes[index].id;
        if (step < hops) {
            if (nodes[index].next < 0) return 1;
            index = (unsigned)nodes[index].next;
        }
    }
    return write_answer(out, cap, values, 2);
}

static uint32_t dispatch(unsigned index, uint32_t x) {
    switch (index) {
        case 0: return (x ^ 0xc4fdu) + (x << 7);
        case 1: return ((x << 5) + 0xd14cu) ^ (5u * x);
        case 2:
        case 4: return (x + 0xac92u) ^ (x >> 3);
        case 3:
        case 7: return ((x >> 2) ^ x) + 0x151cu;
        case 5: return (x >> 2) | (x << 30);
        default: return ((x << 1) | (x >> 31)) ^ 0xd2c2u;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t x = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    x = dispatch((unsigned)(challenge & 7u), x) ^ 0xed66u;
    uint32_t value = dispatch((unsigned)((challenge >> 3) & 7u), x) + 0x8b77u;
    return write_answer(out, cap, &value, 1);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t a = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t b = (uint32_t)context ^ (uint32_t)(context >> 32);
    uint32_t values[3];
    uint32_t x = a ^ b;
    values[0] = (uint32_t)context & 3u;
    values[1] = ((x >> 7) | (x << 25)) ^ 0x9a94u
                ^ (values[0] * 0xe940u) ^ (uint32_t)(context >> 16);
    values[2] = values[1] ^ (b + values[0] + 0x73d4u + (a >> 7));

    if (cap <= 36u) return 1;
    memset(out, 'A', 32);
    memcpy(out + 32, "OPEN", 4);
    return write_answer(out + 36, cap - 36, values, 3);
}
