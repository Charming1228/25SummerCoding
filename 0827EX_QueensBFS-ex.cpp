#include <iostream>

#define MAX_N 8
#define MAX_NODES 10000 * MAX_N

#define put_node(st) queue[tl++] = st
#define get_node() queue[hd++]
#define queue_not_empty (hd < tl)

typedef struct state_t
{
    short n, q[MAX_N]; // n表示已经放置的皇后数量，q数组存储每行皇后的列位置
} state_t;

state_t queue[MAX_NODES];
int hd = 0, tl = 0;

int conflict(int row, int col, short queens[]); // 检查(row,col)位置是否冲突
int queen(int n);                               // 解决n皇后问题
void print_queens(state_t *st);                 // 打印皇后的位置

int main()
{
    queen(MAX_N);
    return 0;
}

void print_queens(state_t *st)
{
    printf("Solution: ");
    for (int i = 0; i < st->n; i++)
    {
        printf("(%d,%d) ", i, st->q[i]);
    }
    printf("\n");

    // 可选：打印棋盘图形
    for (int i = 0; i < st->n; i++)
    {
        for (int j = 0; j < st->n; j++)
        {
            if (st->q[i] == j)
            {
                printf("Q ");
            }
            else
            {
                printf(". ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

// 检查在第row行第col列放置皇后是否与之前的皇后冲突
int conflict(int row, int col, short queens[])
{
    for (int i = 0; i < row; i++)
    {
        // 检查同一列
        if (queens[i] == col)
        {
            return 1;
        }
        // 检查对角线：行差 == 列差
        if (abs(queens[i] - col) == abs(i - row))
        {
            return 1;
        }
    }
    return 0;
}

int queen(int n)
{
    int nCount = 0;
    state_t st;

    // 初始化：放置第一个皇后
    st.n = 0;
    put_node(st);

    while (queue_not_empty)
    {
        st = get_node();

        if (st.n >= n)
        {
            print_queens(&st);
            nCount++;
            continue;
        }

        // 尝试在当前行的每一列放置皇后
        for (int i = 0; i < n; i++)
        {
            if (!conflict(st.n, i, st.q))
            {
                state_t new_st = st;
                new_st.q[new_st.n] = i; // 在当前行的第i列放置皇后
                new_st.n++;             // 移动到下一行
                put_node(new_st);
            }
        }
    }

    printf("Total solutions for %d-queens problem: %d\n", n, nCount);
    return nCount;
}