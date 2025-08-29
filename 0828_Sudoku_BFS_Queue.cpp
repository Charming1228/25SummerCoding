#include <iostream>
#include <vector>
#include <queue>
using namespace std;

// 数独尺寸常量
const int SIZE = 9;
const int SUB_SIZE = 3;

/**
 * @brief 校验数字填入合法性（与 DFS 版本一致）
 * @param board 数独状态
 * @param row 目标行
 * @param col 目标列
 * @param num 待填数字
 * @return 合法返回 true，否则 false
 */
bool isValid(const vector<vector<int>> &board, int row, int col, int num)
{
    // 校验行
    for (int c = 0; c < SIZE; ++c)
    {
        if (board[row][c] == num)
            return false;
    }
    // 校验列
    for (int r = 0; r < SIZE; ++r)
    {
        if (board[r][col] == num)
            return false;
    }
    // 校验 3×3 宫
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
 * @brief 寻找下一个待填空格（行优先，与 DFS 版本一致）
 * @param board 数独状态
 * @param row 输出参数：空格行坐标
 * @param col 输出参数：空格列坐标
 * @return 找到空格返回 true，无空格返回 false
 */
bool findEmpty(const vector<vector<int>> &board, int &row, int &col)
{
    for (row = 0; row < SIZE; ++row)
    {
        for (col = 0; col < SIZE; ++col)
        {
            if (board[row][col] == 0)
                return true;
        }
    }
    return false;
}

/**
 * @brief 格式化打印数独（与 DFS 版本一致）
 * @param board 待打印的数独状态
 */
void printBoard(const vector<vector<int>> &board)
{
    for (int i = 0; i < SIZE; ++i)
    {
        for (int j = 0; j < SIZE; ++j)
        {
            cout << board[i][j];
            if (j != SIZE - 1)
                cout << " ";
        }
        cout << endl;
    }
}

/**
 * @brief BFS 求解数独（核心函数）
 * @param initialBoard 初始数独状态
 * @param result 输出参数：存储求解结果（若有解）
 * @return 有解返回 true，无解返回 false
 */
bool bfsSolve(const vector<vector<int>> &initialBoard, vector<vector<int>> &result)
{
    // 队列元素：pair<当前数独状态, 下一个待填空格的坐标>
    // 用 pair 存储 (row, col)，避免每次出队后重复调用 findEmpty 找空格
    queue<pair<vector<vector<int>>, pair<int, int>>> q;

    // 1. 初始化队列：计算初始状态的第一个空格，入队
    int initRow, initCol;
    if (findEmpty(initialBoard, initRow, initCol))
    {
        q.push({initialBoard, {initRow, initCol}});
    }
    else
    {
        // 初始状态已是终态（无空格）
        result = initialBoard;
        return true;
    }

    // 2. BFS 主循环：处理队列中的所有状态
    while (!q.empty())
    {
        // 出队：获取当前状态和下一个待填空格
        auto current = q.front();
        q.pop();
        vector<vector<int>> currentBoard = current.first;
        int row = current.second.first;
        int col = current.second.second;

        // 3. 尝试为当前空格填入 1-9 的合法数字
        for (int num = 1; num <= SIZE; ++num)
        {
            if (isValid(currentBoard, row, col, num))
            {
                // 生成新状态（复制当前状态，填入合法数字）
                vector<vector<int>> newBoard = currentBoard;
                newBoard[row][col] = num;

                // 4. 检查新状态是否为终态（无空格）
                int nextRow, nextCol;
                if (!findEmpty(newBoard, nextRow, nextCol))
                {
                    // 找到终态，赋值给 result 并返回
                    result = newBoard;
                    return true;
                }

                // 5. 新状态不是终态，入队（记录下一个待填空格）
                q.push({newBoard, {nextRow, nextCol}});
            }
        }
    }

    // 6. 队列为空，无求解
    return false;
}

// ---------------------- 测试用例（文档样例输入） ----------------------
vector<vector<int>> sampleInput = {
    {0, 4, 0, 2, 8, 0, 0, 0, 0},
    {2, 1, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 9, 0, 0, 4, 5, 0, 0},
    {0, 0, 7, 0, 0, 9, 6, 0, 0},
    {0, 0, 4, 3, 0, 0, 0, 8, 0},
    {0, 0, 0, 0, 8, 0, 0, 0, 0},
    {0, 8, 0, 0, 6, 0, 7, 0, 0},
    {0, 0, 0, 0, 2, 0, 1, 0, 0},
    {7, 1, 0, 9, 0, 0, 0, 3, 0}};

// 文档其他初始状态（可替换测试）
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

// ---------------------- 主函数（程序入口） ----------------------
int main()
{
    system("chcp 65001 > nul");
    
    // 选择测试用例（可替换为 testCase1）
    vector<vector<int>> initialBoard = sampleInput;
    vector<vector<int>> result;

    cout << "初始数独：" << endl;
    printBoard(initialBoard);
    cout << endl;

    // 调用 BFS 求解
    if (bfsSolve(initialBoard, result))
    {
        cout << "BFS 求解结果：" << endl;
        printBoard(result);
    }
    else
    {
        cout << "该数独无解！" << endl;
    }

    return 0;
}