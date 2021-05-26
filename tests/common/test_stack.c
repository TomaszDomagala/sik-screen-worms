#include "unity.h"
#include "stack.h"

void setUp() {

}

void tearDown() {

}

void test_stack_1() {
    int n = 200;
    stack_t *stack = stack_create(sizeof(int));

    TEST_ASSERT_EQUAL(0, stack_size(stack));

    for (int i = 0; i < n; ++i) {
        stack_push(stack, &i);
        TEST_ASSERT_EQUAL(i + 1, stack_size(stack));
    }
    for (int i = 0; i < n; ++i) {
        int *j = stack_get(stack, i);
        TEST_ASSERT_EQUAL(i, *j);
    }

    stack_free(stack);
}

typedef struct {
    int64_t x;
    int64_t y;
} test_vector_t;

void test_stack_2() {
    int n = 200;
    stack_t *stack = stack_create(sizeof(test_vector_t));

    TEST_ASSERT_EQUAL(0, stack_size(stack));
    test_vector_t v;

    for (int i = 0; i < n; ++i) {
        v.x = i;
        v.y = i * i;

        stack_push(stack, &v);
        TEST_ASSERT_EQUAL(i + 1, stack_size(stack));
    }
    for (int i = 0; i < n; ++i) {
        test_vector_t *vec = stack_get(stack, i);
        TEST_ASSERT_EQUAL(i, vec->x);
        TEST_ASSERT_EQUAL(i * i, vec->y);
    }

    stack_free(stack);
}


int main() {
    UNITY_BEGIN();

    RUN_TEST(test_stack_1);
    RUN_TEST(test_stack_2);

    return UNITY_END();
}
