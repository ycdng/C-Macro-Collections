#include "utl.c"
#include "utl/assert.h"
#include "utl/log.h"
#include "utl/test.h"

#include "../src/deque.c"

struct deque_fval *d_fval = &(struct deque_fval){ .cmp = cmc_size_cmp,
                                                  .cpy = NULL,
                                                  .str = cmc_size_str,
                                                  .free = NULL,
                                                  .hash = cmc_size_hash,
                                                  .pri = cmc_size_cmp };

struct deque_fval *d_fval_counter = &(struct deque_fval){ .cmp = v_c_cmp,
                                                          .cpy = v_c_cpy,
                                                          .str = v_c_str,
                                                          .free = v_c_free,
                                                          .hash = v_c_hash,
                                                          .pri = v_c_pri };

struct cmc_alloc_node *d_alloc_node = &(struct cmc_alloc_node){
    .malloc = malloc, .calloc = calloc, .realloc = realloc, .free = free
};

CMC_CREATE_UNIT(Deque, true, {
    CMC_CREATE_TEST(PFX##_new(), {
        struct deque *d = d_new(1000000, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);
        cmc_assert_not_equals(ptr, NULL, d->buffer);
        cmc_assert_equals(size_t, 1000000, d->capacity);
        cmc_assert_equals(size_t, 0, d_count(d));

        d_free(d);

        d = d_new(0, d_fval);

        cmc_assert_equals(ptr, NULL, d);

        d = d_new(1000, NULL);

        cmc_assert_equals(ptr, NULL, d);

        d = d_new(UINT64_MAX, d_fval);

        cmc_assert_equals(ptr, NULL, d);

        if (d)
            d_free(d);
    });

    CMC_CREATE_TEST(PFX##_new_custom(), {
        struct deque *d = d_new_custom(100, d_fval, d_alloc_node, callbacks);

        cmc_assert_not_equals(ptr, NULL, d);
        cmc_assert_equals(ptr, d_alloc_node, d->alloc);
        cmc_assert_equals(ptr, callbacks, d->callbacks);

        d_free(d);

        d = d_new_custom(0, d_fval, d_alloc_node, callbacks);

        cmc_assert_equals(ptr, NULL, d);

        d = d_new_custom(1000, NULL, d_alloc_node, callbacks);

        cmc_assert_equals(ptr, NULL, d);

        d = d_new_custom(UINT64_MAX, d_fval, d_alloc_node, callbacks);

        cmc_assert_equals(ptr, NULL, d);

        if (d)
            d_free(d);
    });

    CMC_CREATE_TEST(PFX##_clear(), {
        v_total_free = 0;

        struct deque *d = d_new(100, d_fval_counter);

        cmc_assert_not_equals(ptr, NULL, d);

        for (size_t i = 0; i < 50; i++)
            d_push_back(d, i);

        cmc_assert_equals(size_t, 50, d_count(d));

        d->flag = cmc_flags.ERROR;
        d_clear(d);

        cmc_assert_equals(size_t, 50, v_total_free);
        cmc_assert_equals(int32_t, cmc_flags.OK, d->flag);

        cmc_assert_equals(size_t, 0, d_count(d));
        cmc_assert_equals(size_t, 100, d_capacity(d));

        for (size_t i = 1; i <= 1000; i++)
        {
            if (i % 2 == 0)
                d_push_front(d, i);
            else
                d_push_back(d, i);
        }

        cmc_assert_equals(size_t, 1000, d_count(d));

        size_t sum = 0;
        for (size_t i = d->front, j = 0; j < d->count;
             i = (i + 1) % d->capacity, j++)
            sum += d->buffer[i];

        cmc_assert_equals(size_t, 500500, sum);

        d->flag = cmc_flags.ERROR;
        d_clear(d);

        cmc_assert_equals(size_t, 1050, v_total_free);
        cmc_assert_equals(int32_t, cmc_flags.OK, d->flag);

        sum = 0;
        for (size_t i = 0; i < d->capacity; i++)
            sum += d->buffer[i];

        cmc_assert_equals(size_t, 0, sum);

        d_free(d);

        cmc_assert_equals(size_t, 1050, v_total_free);

        v_total_free = 0;
    });

    CMC_CREATE_TEST(PFX##_free(), {
        v_total_free = 0;

        struct deque *d = d_new(100, d_fval_counter);

        cmc_assert_not_equals(ptr, NULL, d);

        for (size_t i = 1; i <= 1000; i++)
        {
            if (i % 2 == 0)
                d_push_front(d, i);
            else
                d_push_back(d, i);
        }

        cmc_assert_equals(size_t, 1000, d_count(d));

        size_t sum = 0;
        for (size_t i = d->front, j = 0; j < d->count;
             i = (i + 1) % d->capacity, j++)
            sum += d->buffer[i];

        cmc_assert_equals(size_t, 500500, sum);

        d_free(d);

        cmc_assert_equals(size_t, 1000, v_total_free);

        v_total_free = 0;
    });

    CMC_CREATE_TEST(PFX##_customize(), {
        struct deque *d = d_new(100, d_fval);

        d->flag = cmc_flags.ERROR;
        d_customize(d, d_alloc_node, callbacks);

        cmc_assert_equals(ptr, d_alloc_node, d->alloc);
        cmc_assert_equals(ptr, callbacks, d->callbacks);
        cmc_assert_equals(int32_t, cmc_flags.OK, d->flag);

        d->flag = cmc_flags.ERROR;
        d_customize(d, NULL, NULL);

        cmc_assert_equals(ptr, &cmc_alloc_node_default, d->alloc);
        cmc_assert_equals(ptr, NULL, d->callbacks);
        cmc_assert_equals(int32_t, cmc_flags.OK, d->flag);

        d_free(d);
    });

    CMC_CREATE_TEST(PFX##_push_front(), {
        total_create = 0;
        struct deque *d = d_new_custom(100, d_fval, NULL, callbacks);

        cmc_assert_not_equals(ptr, NULL, d);

        cmc_assert(d_push_front(d, 10));
        cmc_assert_equals(size_t, 1, d_count(d));

        cmc_assert_equals(int32_t, 1, total_create);

        d_customize(d, NULL, NULL);

        d_clear(d);

        for (size_t i = 0; i < 150; i++)
            d_push_front(d, i);

        cmc_assert_equals(size_t, 150, d_count(d));
        cmc_assert_greater(size_t, 100, d_capacity(d));

        d_free(d);

        d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        for (size_t i = 1; i <= 1000; i++)
            d_push_front(d, i);

        size_t sum = 0;

        for (size_t i = d->front, j = 0; j < d->count;
             i = (i + 1) % d->capacity, j++)
        {
            sum += d->buffer[i];
        }

        cmc_assert_equals(size_t, 500500, sum);

        d_free(d);

        cmc_assert_equals(int32_t, 1, total_create);
        total_create = 0;
    });

    CMC_CREATE_TEST(PFX##_push_back(), {
        total_create = 0;
        struct deque *d = d_new_custom(100, d_fval, NULL, callbacks);

        cmc_assert_not_equals(ptr, NULL, d);

        cmc_assert(d_push_front(d, 10));
        cmc_assert_equals(size_t, 1, d_count(d));

        cmc_assert_equals(int32_t, 1, total_create);

        d_customize(d, NULL, NULL);

        d_clear(d);

        for (size_t i = 0; i < 150; i++)
            d_push_back(d, i);

        cmc_assert_equals(size_t, 150, d_count(d));
        cmc_assert_greater(size_t, 100, d_capacity(d));

        d_free(d);

        d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        for (size_t i = 1; i <= 1000; i++)
            d_push_back(d, i);

        size_t sum = 0;

        for (size_t i = d->front, j = 0; j < d->count;
             i = (i + 1) % d->capacity, j++)
        {
            sum += d->buffer[i];
        }

        cmc_assert_equals(size_t, 500500, sum);

        d_free(d);

        cmc_assert_equals(int32_t, 1, total_create);
        total_create = 0;
    });

    CMC_CREATE_TEST(PFX##_pop_front(), {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        cmc_assert(d_push_front(d, 10));
        cmc_assert(d_pop_front(d));

        cmc_assert_equals(size_t, 0, d_count(d));

        cmc_assert(d_push_back(d, 10));
        cmc_assert(d_pop_front(d));

        cmc_assert_equals(size_t, 0, d_count(d));

        d_free(d);

        d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        for (size_t i = 1; i <= 200; i++)
            cmc_assert(d_push_front(d, i));

        for (size_t i = 0; i < 100; i++)
            cmc_assert(d_pop_front(d));

        size_t sum = 0;

        for (size_t i = d->front, j = 0; j < d->count;
             i = (i + 1) % d->capacity, j++)
        {
            sum += d->buffer[i];
        }

        cmc_assert_equals(size_t, 5050, sum);

        d_free(d);
    });

    CMC_CREATE_TEST(pop_back[count capacity], {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        cmc_assert(d_push_front(d, 10));
        cmc_assert(d_pop_back(d));

        cmc_assert_equals(size_t, 0, d_count(d));

        cmc_assert(d_push_back(d, 10));
        cmc_assert(d_pop_back(d));

        cmc_assert_equals(size_t, 0, d_count(d));

        d_free(d);
    });

    CMC_CREATE_TEST(pop_back[item preservation], {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        for (size_t i = 1; i <= 200; i++)
            cmc_assert(d_push_back(d, i));

        for (size_t i = 0; i < 100; i++)
            cmc_assert(d_pop_back(d));

        size_t sum = 0;

        for (size_t i = d->front, j = 0; j < d->count;
             i = (i + 1) % d->capacity, j++)
        {
            sum += d->buffer[i];
        }

        cmc_assert_equals(size_t, 5050, sum);

        d_free(d);
    });

    CMC_CREATE_TEST(Mixed IO, {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        cmc_assert(d_push_front(d, 10));
        cmc_assert(d_push_front(d, 10));
        cmc_assert(d_pop_back(d));
        cmc_assert(d_pop_back(d));

        cmc_assert_equals(size_t, 0, d_count(d));

        cmc_assert(d_push_back(d, 10));
        cmc_assert(d_push_back(d, 10));
        cmc_assert(d_pop_back(d));
        cmc_assert(d_pop_back(d));

        cmc_assert_equals(size_t, 0, d_count(d));

        cmc_assert(d_push_front(d, 10));
        cmc_assert(d_push_front(d, 10));
        cmc_assert(d_pop_front(d));
        cmc_assert(d_pop_front(d));

        cmc_assert_equals(size_t, 0, d_count(d));

        cmc_assert(d_push_back(d, 10));
        cmc_assert(d_push_back(d, 10));
        cmc_assert(d_pop_front(d));
        cmc_assert(d_pop_front(d));

        cmc_assert_equals(size_t, 0, d_count(d));
        // ----------
        cmc_assert(d_push_back(d, 10));
        cmc_assert(d_push_front(d, 10));
        cmc_assert(d_pop_back(d));
        cmc_assert(d_pop_front(d));

        cmc_assert_equals(size_t, 0, d_count(d));

        cmc_assert(d_push_back(d, 10));
        cmc_assert(d_push_front(d, 10));
        cmc_assert(d_pop_front(d));
        cmc_assert(d_pop_back(d));

        cmc_assert_equals(size_t, 0, d_count(d));

        cmc_assert(d_push_front(d, 10));
        cmc_assert(d_push_back(d, 10));
        cmc_assert(d_pop_back(d));
        cmc_assert(d_pop_front(d));

        cmc_assert_equals(size_t, 0, d_count(d));

        cmc_assert(d_push_front(d, 10));
        cmc_assert(d_push_back(d, 10));
        cmc_assert(d_pop_front(d));
        cmc_assert(d_pop_back(d));

        cmc_assert_equals(size_t, 0, d_count(d));

        d_free(d);
    });

    CMC_CREATE_TEST(front[count = 0], {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        size_t r = d_front(d);

        cmc_assert_equals(size_t, (size_t){ 0 }, r);

        d_free(d);
    });

    CMC_CREATE_TEST(front[sum], {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        size_t sum = 0;

        for (size_t i = 0; i <= 100; i++)
        {
            if (i % 2 == 0)
                cmc_assert(d_push_back(d, i));
            else
                cmc_assert(d_push_front(d, i));
        }

        while (!d_empty(d))
        {
            size_t r = d_front(d);

            sum += r;

            cmc_assert(d_pop_front(d));
        }

        cmc_assert_equals(size_t, 5050, sum);

        d_free(d);
    });

    CMC_CREATE_TEST(back[count = 0], {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        size_t r = d_back(d);

        cmc_assert_equals(size_t, (size_t){ 0 }, r);

        d_free(d);
    });

    CMC_CREATE_TEST(back[sum], {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        size_t sum = 0;

        for (size_t i = 0; i <= 100; i++)
        {
            if (i % 2 == 0)
                cmc_assert(d_push_back(d, i));
            else
                cmc_assert(d_push_front(d, i));
        }

        while (!d_empty(d))
        {
            size_t r = d_back(d);

            sum += r;

            cmc_assert(d_pop_back(d));
        }

        cmc_assert_equals(size_t, 5050, sum);

        d_free(d);
    });

    CMC_CREATE_TEST(contains, {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        for (size_t i = 1; i <= 20; i++)
            cmc_assert(d_push_back(d, i));

        for (size_t i = 1; i <= 20; i++)
            cmc_assert(d_contains(d, i));

        d_clear(d);

        for (size_t i = 1; i <= 20; i++)
            cmc_assert(d_push_front(d, i));

        for (size_t i = 1; i <= 20; i++)
            cmc_assert(d_contains(d, i));

        d_free(d);
    });

    CMC_CREATE_TEST(contains[count = 0], {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        cmc_assert(!d_contains(d, 10));

        d_free(d);
    });

    CMC_CREATE_TEST(contains[count = 1], {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        cmc_assert(!d_contains(d, 10));

        cmc_assert(d_push_back(d, 100));

        cmc_assert(d_contains(d, 100));
        cmc_assert(!d_contains(d, 101));

        cmc_assert(d_pop_front(d));
        cmc_assert(d_push_front(d, 100));

        cmc_assert(d_contains(d, 100));
        cmc_assert(!d_contains(d, 101));

        d_free(d);
    });

    CMC_CREATE_TEST(empty, {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        cmc_assert(d_empty(d));

        cmc_assert(d_push_front(d, 1));

        cmc_assert(!d_empty(d));

        d_free(d);
    });

    CMC_CREATE_TEST(full, {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        cmc_assert(!d_full(d));

        for (size_t i = 0; i != d_capacity(d); i++)
            cmc_assert(d_push_front(d, i));

        cmc_assert(d_full(d));

        d_free(d);
    });

    CMC_CREATE_TEST(count, {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        cmc_assert_equals(size_t, 0, d_count(d));

        for (size_t i = 0; i < 100; i++)
            cmc_assert(d_push_back(d, i));

        cmc_assert_equals(size_t, 100, d_count(d));

        d_free(d);
    });

    CMC_CREATE_TEST(capacity, {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        cmc_assert_equals(size_t, 100, d_capacity(d));

        d_free(d);
    });

    CMC_CREATE_TEST(resize, {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        for (size_t i = 0; i < 150; i++)
        {
            if (i % 2 == 0)
                cmc_assert(d_push_back(d, i));
            else
                cmc_assert(d_push_front(d, i));
        }

        cmc_assert_equals(size_t, 150, d_count(d));
        cmc_assert(d_resize(d, d_count(d)));
        cmc_assert_equals(size_t, 150, d_capacity(d));

        d_free(d);
    });

    CMC_CREATE_TEST(resize[item preservation], {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        for (size_t i = 1; i <= 150; i++)
        {
            if (i % 2 != 0)
                cmc_assert(d_push_back(d, i));
            else
                cmc_assert(d_push_front(d, i));
        }

        cmc_assert(d_resize(d, d_count(d)));

        size_t sum = 0;

        for (size_t i = d->front, j = 0; j < d->count;
             i = (i + 1) % d->capacity, j++)
        {
            sum += d->buffer[i];
        }

        cmc_assert_equals(size_t, 11325, sum);

        d_free(d);
    });

    CMC_CREATE_TEST(copy_of, {
        struct deque *d1 = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d1);

        for (size_t i = 1; i <= 150; i++)
        {
            if (i % 2 != 0)
                cmc_assert(d_push_back(d1, i));
            else
                cmc_assert(d_push_front(d1, i));
        }

        struct deque *d2 = d_copy_of(d1);

        cmc_assert_not_equals(ptr, NULL, d2);
        cmc_assert_equals(size_t, d_count(d1), d_count(d2));

        for (size_t i1 = d1->front, i2 = d2->front, j = 0; j < d1->count; j++)
        {
            cmc_assert_equals(size_t, d1->buffer[i1], d2->buffer[i2]);

            i1 = (i1 + 1) % d1->capacity;
            i2 = (i2 + 1) % d2->capacity;
        }

        cmc_assert(d_push_front(d1, 10));
        cmc_assert(d_push_front(d2, 10));
        cmc_assert(d_push_back(d1, 11));
        cmc_assert(d_push_back(d2, 11));

        cmc_assert(d_equals(d1, d2));

        d_free(d1);
        d_free(d2);
    });

    CMC_CREATE_TEST(equals, {
        struct deque *d1 = d_new(100, d_fval);
        struct deque *d2 = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d1);
        cmc_assert_not_equals(ptr, NULL, d2);

        for (size_t i = 1; i <= 150; i++)
        {
            if (i % 2 != 0)
            {
                cmc_assert(d_push_back(d1, i));
                cmc_assert(d_push_back(d2, i));
            }
            else
            {
                cmc_assert(d_push_front(d1, i));
                cmc_assert(d_push_front(d2, i));
            }
        }

        cmc_assert(d_equals(d1, d2));

        d_free(d1);
        d_free(d2);
    });

    CMC_CREATE_TEST(equals[from copy], {
        struct deque *d1 = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d1);

        for (size_t i = 1; i <= 150; i++)
        {
            if (i % 2 != 0)
                cmc_assert(d_push_back(d1, i));
            else
                cmc_assert(d_push_front(d1, i));
        }

        struct deque *d2 = d_copy_of(d1);

        cmc_assert_not_equals(ptr, NULL, d2);

        cmc_assert(d_equals(d1, d2));

        d_free(d1);
        d_free(d2);
    });

    CMC_CREATE_TEST(equals[count = 0], {
        struct deque *d1 = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d1);

        struct deque *d2 = d_copy_of(d1);

        cmc_assert_not_equals(ptr, NULL, d2);

        cmc_assert(d_equals(d1, d2));

        d_free(d1);
        d_free(d2);
    });

    CMC_CREATE_TEST(buffer_growth[capacity = 1], {
        struct deque *d = d_new(1, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        for (size_t i = 0; i < 50; i++)
            d_push_back(d, i);

        cmc_assert_equals(size_t, 50, d_count(d));
        cmc_assert_lesser_equals(size_t, d_capacity(d), d_count(d));

        d_free(d);
    });

    CMC_CREATE_TEST(buffer_growth[item preservation], {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);

        for (size_t i = 1; i <= 1200; i++)
        {
            if (i % 2 == 0)
                d_push_front(d, i);
            else
                d_push_back(d, i);
        }

        size_t sum = 0;
        for (size_t i = d->front, j = 0; j < d->count;
             i = (i + 1) % d->capacity, j++)
            sum += d->buffer[i];

        cmc_assert_equals(size_t, 1200, d_count(d));
        cmc_assert_equals(size_t, 720600, sum);

        d_free(d);
    });

    CMC_CREATE_TEST(flags, {
        struct deque *d = d_new(100, d_fval);

        cmc_assert_not_equals(ptr, NULL, d);
        cmc_assert_equals(int32_t, cmc_flags.OK, d_flag(d));

        // customize
        d->flag = cmc_flags.ERROR;
        d_customize(d, &cmc_alloc_node_default, &(struct cmc_callbacks){ 0 });
        cmc_assert_equals(int32_t, cmc_flags.OK, d_flag(d));

        // push_front
        d->flag = cmc_flags.ERROR;
        cmc_assert(d_push_front(d, 1));
        cmc_assert_equals(int32_t, cmc_flags.OK, d_flag(d));

        // pop_front
        d->flag = cmc_flags.ERROR;
        cmc_assert(d_pop_front(d));
        cmc_assert_equals(int32_t, cmc_flags.OK, d_flag(d));
        cmc_assert(!d_pop_front(d));
        cmc_assert_equals(int32_t, cmc_flags.EMPTY, d_flag(d));

        // push_back
        cmc_assert(d_push_back(d, 1));
        cmc_assert_equals(int32_t, cmc_flags.OK, d_flag(d));

        // pop_back
        d->flag = cmc_flags.ERROR;
        cmc_assert(d_pop_back(d));
        cmc_assert_equals(int32_t, cmc_flags.OK, d_flag(d));
        cmc_assert(!d_pop_back(d));
        cmc_assert_equals(int32_t, cmc_flags.EMPTY, d_flag(d));

        // front
        d->flag = cmc_flags.ERROR;
        cmc_assert(d_front(d) == 0);
        cmc_assert_equals(int32_t, cmc_flags.EMPTY, d_flag(d));

        cmc_assert(d_push_back(d, 1));
        d->flag = cmc_flags.ERROR;
        cmc_assert(d_front(d) == 1);
        cmc_assert_equals(int32_t, cmc_flags.OK, d_flag(d));

        // clear
        d->flag = cmc_flags.ERROR;
        d_clear(d);
        cmc_assert_equals(int32_t, cmc_flags.OK, d_flag(d));

        // back
        cmc_assert(d_back(d) == 0);
        cmc_assert_equals(int32_t, cmc_flags.EMPTY, d_flag(d));

        cmc_assert(d_push_front(d, 1));
        cmc_assert(d_back(d) == 1);
        cmc_assert_equals(int32_t, cmc_flags.OK, d_flag(d));

        // contains
        d->flag = cmc_flags.ERROR;
        cmc_assert(d_contains(d, 1));
        cmc_assert_equals(int32_t, cmc_flags.OK, d_flag(d));

        // copy_of
        d->flag = cmc_flags.ERROR;
        struct deque *d2 = d_copy_of(d);
        cmc_assert_equals(int32_t, cmc_flags.OK, d_flag(d));
        cmc_assert_equals(int32_t, cmc_flags.OK, d_flag(d2));

        // equals
        cmc_assert(d_pop_back(d));
        cmc_assert(d_pop_back(d2));
        d->flag = cmc_flags.ERROR;
        d2->flag = cmc_flags.ERROR;
        d_front(d);
        d_front(d2);
        cmc_assert_equals(int32_t, cmc_flags.EMPTY, d_flag(d));
        cmc_assert_equals(int32_t, cmc_flags.EMPTY, d_flag(d2));
        cmc_assert(d_equals(d, d2));
        cmc_assert_equals(int32_t, cmc_flags.OK, d_flag(d));
        cmc_assert_equals(int32_t, cmc_flags.OK, d_flag(d2));

        d_free(d);
        d_free(d2);
    });

    CMC_CREATE_TEST(callbacks, {
        struct deque *d = d_new_custom(100, d_fval, NULL, callbacks);

        total_create = 0;
        total_read = 0;
        total_update = 0;
        total_delete = 0;
        total_resize = 0;

        cmc_assert(d_push_front(d, 10));
        cmc_assert_equals(int32_t, 1, total_create);

        cmc_assert(d_push_back(d, 10));
        cmc_assert_equals(int32_t, 2, total_create);

        cmc_assert(d_pop_front(d));
        cmc_assert_equals(int32_t, 1, total_delete);

        cmc_assert(d_pop_back(d));
        cmc_assert_equals(int32_t, 2, total_delete);

        cmc_assert(d_push_front(d, 10));
        cmc_assert(d_push_front(d, 5));
        cmc_assert_equals(int32_t, 4, total_create);

        cmc_assert_equals(size_t, 5, d_front(d));
        cmc_assert_equals(int32_t, 1, total_read);

        cmc_assert_equals(size_t, 10, d_back(d));
        cmc_assert_equals(int32_t, 2, total_read);

        cmc_assert(d_contains(d, 10));
        cmc_assert(!d_contains(d, 1));
        cmc_assert_equals(int32_t, 4, total_read);

        cmc_assert(d_resize(d, 1000));
        cmc_assert_equals(int32_t, 1, total_resize);

        cmc_assert(d_resize(d, 10));
        cmc_assert_equals(int32_t, 2, total_resize);

        cmc_assert_equals(int32_t, 4, total_create);
        cmc_assert_equals(int32_t, 4, total_read);
        cmc_assert_equals(int32_t, 0, total_update);
        cmc_assert_equals(int32_t, 2, total_delete);
        cmc_assert_equals(int32_t, 2, total_resize);

        d_customize(d, NULL, NULL);

        cmc_assert_equals(ptr, NULL, d->callbacks);

        d_clear(d);
        cmc_assert(d_push_front(d, 10));
        cmc_assert(d_push_back(d, 10));
        cmc_assert(d_pop_front(d));
        cmc_assert(d_pop_back(d));
        cmc_assert(d_push_front(d, 10));
        cmc_assert(d_push_front(d, 5));
        cmc_assert_equals(size_t, 5, d_front(d));
        cmc_assert_equals(size_t, 10, d_back(d));
        cmc_assert(d_contains(d, 10));
        cmc_assert(!d_contains(d, 1));
        cmc_assert(d_resize(d, 1000));
        cmc_assert(d_resize(d, 10));

        cmc_assert_equals(int32_t, 4, total_create);
        cmc_assert_equals(int32_t, 4, total_read);
        cmc_assert_equals(int32_t, 0, total_update);
        cmc_assert_equals(int32_t, 2, total_delete);
        cmc_assert_equals(int32_t, 2, total_resize);

        cmc_assert_equals(ptr, NULL, d->callbacks);

        d_free(d);

        total_create = 0;
        total_read = 0;
        total_update = 0;
        total_delete = 0;
        total_resize = 0;
    });
});
