#include "unity.h"

#include "game_board.h"
#include <stdbool.h>

void setUp() {

}

void tearDown() {

}

void test_board_1() {
    game_board_t *board = game_board_create(8, 8);
    TEST_ASSERT_NOT_NULL(board);

    TEST_ASSERT_FALSE(game_board_get(board, 0, 0));
    game_board_set(board, 0, 0);
    TEST_ASSERT_TRUE(game_board_get(board, 0, 0));

    TEST_ASSERT_FALSE(game_board_get(board, 7, 7));
    game_board_set(board, 7, 7);
    TEST_ASSERT_TRUE(game_board_get(board, 7, 7));

    game_board_free(board);
}

void test_board_empty() {
    int w = 15, h = 17;

    game_board_t *board = game_board_create(w, h);
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            TEST_ASSERT_FALSE(game_board_get(board, j, i));
        }
    }
    game_board_free(board);
}

void test_board_set_all() {
    int w = 15, h = 17;

    game_board_t *board = game_board_create(w, h);
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            game_board_set(board, j, i);
        }
    }
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            TEST_ASSERT_TRUE(game_board_get(board, j, i));
        }
    }
    game_board_free(board);
}

void test_board_2() {
    game_board_t *board = game_board_create(6, 2);
    game_board_set(board, 1, 1);
    game_board_set(board, 2, 1);

    for (int y = 0; y < 2; ++y) {
        for (int x = 0; x < 6; ++x) {
            if (x == 1 && y == 1 || x == 2 && y == 1) {
                TEST_ASSERT_TRUE(game_board_get(board, x, y));
            } else {
                TEST_ASSERT_FALSE(game_board_get(board, x, y));
            }
        }
    }
    game_board_free(board);
}


int main() {
    UNITY_BEGIN();
    RUN_TEST(test_board_empty);
    RUN_TEST(test_board_set_all);
    RUN_TEST(test_board_1);
    RUN_TEST(test_board_2);
    return UNITY_END();
}
