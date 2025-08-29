#include <iostream>
#include <vector>
using namespace std;

// 全局常量：数独尺寸（9×9）
const int SIZE = 9;
// 3×3宫的尺寸
const int SUB_SIZE = 3;

/**
 * @brief 校验数字填入合法性
 * @param board 数独盘面
 * @param row 目标行坐标
 * @param col 目标列坐标
 * @param num 待填入数字（1-9）
 * @return 合法返回true，否则返回false
 */
bool isValid(const vector<vector<int>> &board, int row, int col, int num)
{
    // 1. 校验当前行：是否已存在num
    for (int c = 0; c < SIZE; ++c)
    {
        if (board[row][c] == num)
        {
            return false;
        }
    }

    // 2. 校验当前列：是否已存在num
    for (int r = 0; r < SIZE; ++r)
    {
        if (board[r][col] == num)
        {
            return false;
        }
    }

    // 3. 校验当前3×3宫：计算宫的起始坐标
    int startRow = (row / SUB_SIZE) * SUB_SIZE;
    int startCol = (col / SUB_SIZE) * SUB_SIZE;
    for (int i = startRow; i < startRow + SUB_SIZE; ++i)
    {
        for (int j = startCol; j < startCol + SUB_SIZE; ++j)
        {
            if (board[i][j] == num)
            {
                return false;
            }
        }
    }

    return true; // 三重校验通过，数字合法
}

/**
 * @brief 寻找下一个空格（行优先：从上到下、从左到右）
 * @param board 数独盘面
 * @param row 输出参数：空格的行坐标
 * @param col 输出参数：空格的列坐标
 * @return 找到空格返回true，无空格（已解完）返回false
 */
bool findEmpty(const vector<vector<int>> &board, int &row, int &col)
{
    for (row = 0; row < SIZE; ++row)
    {
        for (col = 0; col < SIZE; ++col)
        {
            if (board[row][col] == 0)
            {
                return true;
            }
        }
    }
    return false; // 无空格，数独已解
}

/**
 * @brief DFS回溯求解数独（核心函数）
 * @param board 数独盘面（引用传递，直接修改）
 * @return 找到解返回true，无解返回false
 */
bool dfsSolve(vector<vector<int>> &board)
{
    int row, col;
    // 终止条件：无空格 → 解成功
    if (!findEmpty(board, row, col))
    {
        return true;
    }

    // 尝试填入1-9的每个数字
    for (int num = 1; num <= SIZE; ++num)
    {
        // 校验数字合法性
        if (isValid(board, row, col, num))
        {
            board[row][col] = num; // 填入合法数字

            // 递归求解：若后续路径找到解，直接返回true（终止递归）
            if (dfsSolve(board))
            {
                return true;
            }

            // 回溯：当前数字无效，恢复为空格（0），尝试下一个数字
            board[row][col] = 0;
        }
    }

    // 1-9均无效，返回false，通知上层递归更换数字
    return false;
}

/**
 * @brief 格式化打印数独（与样例输出格式一致）
 * @param board 数独盘面
 */
void printBoard(const vector<vector<int>> &board)
{
    for (int i = 0; i < SIZE; ++i)
    {
        for (int j = 0; j < SIZE; ++j)
        {
            cout << board[i][j];
            if (j != SIZE - 1)
            { // 非最后一列，打印空格分隔
                cout << " ";
            }
        }
        cout << endl; // 每行结束换行
    }
}

// ---------------------- 测试用例（文档中的初始状态） ----------------------
// 测试用例1（文档第一个初始状态）
vector<vector<int>> testCase1 = {
    {8, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 3, 6, 0, 0, 0, 0, 0},
    {0, 7, 0, 0, 9, 0, 2, 0, 0},
    {0, 5, 0, 0, 0, 7, 0, 0, 0},
    {0, 0, 0, 0, 4, 5, 7, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 3, 0},
    {0, 0, 1, 0, 0, 0, 0, 6, 8},
    {0, 0, 8, 5, 0, 0, 0, 1, 0},
    {0, 9, 0, 0, 0, 0, 4, 0, 0}};

// 测试用例2（文档第二个初始状态）
vector<vector<int>> testCase2 = {
    {0, 2, 0, 0, 0, 9, 0, 1, 0},
    {5, 0, 6, 0, 0, 0, 3, 0, 9},
    {0, 8, 0, 5, 0, 2, 0, 6, 0},
    {0, 0, 5, 0, 7, 0, 1, 0, 0},
    {0, 0, 0, 2, 0, 8, 0, 0, 0},
    {0, 0, 4, 0, 1, 0, 8, 0, 0},
    {0, 5, 0, 8, 0, 7, 0, 3, 0},
    {7, 0, 2, 3, 0, 0, 4, 0, 5},
    {0, 4, 0, 0, 0, 0, 0, 7, 0}};

// 测试用例3（文档第三个初始状态）
vector<vector<int>> testCase3 = {
    {0, 6, 0, 5, 9, 3, 0, 0, 0},
    {9, 0, 1, 0, 0, 0, 5, 0, 0},
    {0, 3, 0, 4, 0, 0, 0, 9, 0},
    {1, 0, 8, 0, 2, 0, 0, 0, 4},
    {4, 0, 0, 3, 0, 9, 0, 0, 1},
    {2, 0, 0, 0, 1, 0, 6, 0, 9},
    {0, 8, 0, 0, 0, 6, 0, 2, 0},
    {0, 0, 4, 0, 0, 0, 8, 0, 7},
    {0, 0, 0, 7, 8, 5, 0, 1, 0}};

// 文档样例输入（用于验证输出一致性）
vector<vector<int>> sampleInput = {
    {0, 0, 0, 3, 9, 0, 0, 0, 5},
    {9, 0, 0, 0, 0, 0, 0, 4, 1},
    {0, 0, 8, 0, 4, 6, 9, 0, 3},
    {4, 7, 6, 0, 0, 0, 0, 5, 0},
    {0, 0, 2, 6, 0, 3, 4, 0, 0},
    {0, 8, 0, 0, 0, 0, 2, 1, 6},
    {8, 0, 4, 5, 1, 0, 7, 0, 0},
    {2, 1, 0, 0, 0, 0, 0, 0, 4},
    {6, 0, 0, 0, 3, 7, 0, 0, 0}};

// ---------------------- 主函数（程序入口） ----------------------
int main()
{
    // 选择测试用例（可替换为testCase1/testCase2/testCase3/sampleInput）
    //vector<vector<int>> currentTest = sampleInput;
    vector<vector<int>> currentTest = testCase1;
    //vector<vector<int>> currentTest = testCase2;
    //vector<vector<int>> currentTest = testCase3;

    cout << "初始数独：" << endl;
    printBoard(currentTest);
    cout << endl; // 空行分隔初始状态与结果

    // 调用DFS求解
    if (dfsSolve(currentTest))
    {
        cout << "求解结果：" << endl;
        printBoard(currentTest);
    }
    else
    {
        cout << "该数独无解！" << endl;
    }

    return 0;
}