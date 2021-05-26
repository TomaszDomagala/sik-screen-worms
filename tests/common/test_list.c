#include "unity.h"
#include "list.h"

void setUp() {

}

void tearDown() {

}

void test_create_list() {
	list_t *list = list_create(sizeof(int));

	TEST_ASSERT_NOT_NULL(list);
	TEST_ASSERT_EQUAL_size_t(0, list_size(list));
	list_free(list);
}

void test_add_remove_list() {
	list_t *list = list_create(sizeof(int));

	int num = 13;

	list_node_t *node = list_add(list, &num);

	TEST_ASSERT_EQUAL_size_t(1, list_size(list));
	TEST_ASSERT_EQUAL(13, *((int *) list_element(node)));

	list_remove(list, node);
	TEST_ASSERT_EQUAL_size_t(0, list_size(list));

	list_free(list);
}

void test_remove_head() {
	list_t *list = list_create(sizeof(int));

	for (int i = 0; i < 32; ++i) {
		list_add(list, &i);
	}

	TEST_ASSERT_EQUAL_size_t(32, list_size(list));
	for (int i = 0; i < 32; ++i) {
		list_node_t *head = list_head(list);
		TEST_ASSERT_EQUAL(i, *((int *) list_element(head)));
		list_remove(list,head);
	}

	list_free(list);
}


int main() {
	UNITY_BEGIN();
	RUN_TEST(test_create_list);
	RUN_TEST(test_add_remove_list);
	RUN_TEST(test_remove_head);
	return UNITY_END();
}
