#include <iostream>
#include <vector>
#include <stack>
using namespace std;

const int SIZE = 9;
const int SUB_SIZE = 3;

/**
 * @brief 校验数字填入合法性
 */
bool isValid(const vector<vector<int>> &board, int row, int col, int num)
{
    // 检查行
    for (int c = 0; c < SIZE; ++c)
    {
        if (board[row][c] == num)
            return false;
    }

    // 检查列
    for (int r = 0; r < SIZE; ++r)
    {
        if (board[r][col] == num)
            return false;
    }

    // 检查3×3宫
    int startRow = (row / SUB_SIZE) * SUB_SIZE;
    int startCol = (col / SUB_SIZE) * SUB_SIZE;
    for (int i = startRow; i < startRow + SUB_SIZE; ++i)
    {
        for (int j = startCol; j < startCol + SUB_SIZE; ++j)
        {
            if (board[i][j] == num)
                return false;
        }
    }

    return true;
}

/**
 * @brief 寻找下一个空格（从指定位置开始）
 */
bool findNextEmpty(const vector<vector<int>> &board, int &row, int &col)
{
    for (int r = row; r < SIZE; ++r)
    {
        int startCol = (r == row) ? col + 1 : 0;
        for (int c = startCol; c < SIZE; ++c)
        {
            if (board[r][c] == 0)
            {
                row = r;
                col = c;
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief 使用栈实现的DFS求解数独（非递归）
 */
bool stackSolve(vector<vector<int>> &board)
{
    stack<pair<int, int>> posStack; // 存储位置 (row, col)
    stack<int> numStack;            // 存储当前尝试的数字

    int row = 0, col = 0;

    // 找到第一个空格
    bool found = false;
    for (row = 0; row < SIZE && !found; ++row)
    {
        for (col = 0; col < SIZE && !found; ++col)
        {
            if (board[row][col] == 0)
            {
                found = true;
                break;
            }
        }
        if (found)
            break;
    }

    if (!found)
        return true; // 没有空格，直接返回

    // 初始化栈
    posStack.push({row, col});
    numStack.push(0); // 从0开始，后面会+1

    while (!posStack.empty())
    {
        auto [currentRow, currentCol] = posStack.top();
        int &currentNum = numStack.top();

        // 如果当前位置已经有数字，先清除（回溯时）
        if (board[currentRow][currentCol] != 0 && currentNum > 0)
        {
            board[currentRow][currentCol] = 0;
        }

        // 尝试下一个数字
        currentNum++;

        // 如果所有数字都尝试过了，回溯
        if (currentNum > SIZE)
        {
            posStack.pop();
            numStack.pop();
            if (!posStack.empty())
            {
                board[currentRow][currentCol] = 0; // 清除当前数字
            }
            continue;
        }

        // 检查当前数字是否合法
        if (isValid(board, currentRow, currentCol, currentNum))
        {
            board[currentRow][currentCol] = currentNum;

            // 寻找下一个空格
            int nextRow = currentRow;
            int nextCol = currentCol;
            if (findNextEmpty(board, nextRow, nextCol))
            {
                // 还有空格，继续处理
                posStack.push({nextRow, nextCol});
                numStack.push(0);
            }
            else
            {
                // 没有空格了，求解完成
                return true;
            }
        }
    }

    return false; // 栈空且未找到解
}

/**
 * @brief 格式化打印数独
 */
void printBoard(const vector<vector<int>> &board)
{
    for (int i = 0; i < SIZE; ++i)
    {
        for (int j = 0; j < SIZE; ++j)
        {
            cout << board[i][j];
            if (j != SIZE - 1)
            {
                cout << " ";
            }
        }
        cout << endl;
    }
}

// 测试用例
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

int main()
{
    system("chcp 65001 > nul");

    vector<vector<int>> sudoku = sampleInput;

    cout << "初始数独：" << endl;
    printBoard(sudoku);
    cout << endl;

    if (stackSolve(sudoku))
    {
        cout << "求解结果（使用栈实现）：" << endl;
        printBoard(sudoku);
    }
    else
    {
        cout << "该数独无解！" << endl;
    }

    return 0;
}