#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
using namespace std;

const int SIZE = 9;
const int SUB_SIZE = 3;

// ---------------- 工具函数 ----------------

// 检查 num 是否可以填入 (row,col)
bool isValid(const vector<vector<int>> &board, int row, int col, int num) {
    for (int c = 0; c < SIZE; c++) if (board[row][c] == num) return false;
    for (int r = 0; r < SIZE; r++) if (board[r][col] == num) return false;

    int startRow = (row / SUB_SIZE) * SUB_SIZE;
    int startCol = (col / SUB_SIZE) * SUB_SIZE;
    for (int i = startRow; i < startRow + SUB_SIZE; i++)
        for (int j = startCol; j < startCol + SUB_SIZE; j++)
            if (board[i][j] == num) return false;
    return true;
}

// 找下一个空格 (行优先)
bool findEmpty(const vector<vector<int>> &board, int &row, int &col) {
    for (row = 0; row < SIZE; row++)
        for (col = 0; col < SIZE; col++)
            if (board[row][col] == 0) return true;
    return false;
}

// 找候选数最少的格子 (MRV)
bool findMRV(const vector<vector<int>> &board, int &row, int &col) {
    int best = 10;
    bool found = false;
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (board[r][c] == 0) {
                int cnt = 0;
                for (int num = 1; num <= 9; num++)
                    if (isValid(board, r, c, num)) cnt++;
                if (cnt < best) {
                    best = cnt;
                    row = r; col = c;
                    found = true;
                }
            }
        }
    }
    return found;
}

// 打印数独棋盘
void printBoard(const vector<vector<int>> &board) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            cout << board[i][j];
            if (j != SIZE - 1) cout << " ";
        }
        cout << endl;
    }
}

// ---------------- 普通 BFS ----------------
bool bfsSolve(const vector<vector<int>> &initialBoard, vector<vector<int>> &result, int &expanded) {
    queue<pair<vector<vector<int>>, pair<int,int>>> q;

    int row, col;
    if (findEmpty(initialBoard, row, col)) {
        q.push({initialBoard, {row, col}});
    } else {
        result = initialBoard;
        return true;
    }

    expanded = 0; // 初始化计数器

    while (!q.empty()) {
        auto cur = q.front(); q.pop();
        expanded++;  // 每出队一个节点就算一次扩展

        auto board = cur.first;
        int r = cur.second.first, c = cur.second.second;

        for (int num = 1; num <= 9; num++) {
            if (isValid(board, r, c, num)) {
                auto newBoard = board;
                newBoard[r][c] = num;

                int nr, nc;
                if (!findEmpty(newBoard, nr, nc)) {
                    result = newBoard;
                    return true;
                }
                q.push({newBoard, {nr,nc}});
            }
        }
    }
    return false;
}

// ---------------- 启发式 BFS (Best-First) ----------------

// 评分函数：所有空格候选数之和 (越小越好)
int heuristic(const vector<vector<int>> &board) {
    int total = 0;
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (board[r][c] == 0) {
                int cnt = 0;
                for (int num = 1; num <= 9; num++)
                    if (isValid(board,r,c,num)) cnt++;
                total += cnt;
            }
        }
    }
    return total;
}

struct Node {
    vector<vector<int>> board;
    int row, col;
    int score;
    bool operator<(const Node& other) const {
        return score > other.score; // 小分优先
    }
};

bool heuristicSolve(const vector<vector<int>> &initialBoard, vector<vector<int>> &result, int &expanded) {
    priority_queue<Node> pq;
    int row, col;
    if (!findMRV(initialBoard, row, col)) {
        result = initialBoard;
        return true;
    }
    pq.push({initialBoard, row, col, heuristic(initialBoard)});
    expanded = 0;

    while (!pq.empty()) {
        Node cur = pq.top(); pq.pop();
        expanded++;

        auto board = cur.board;
        int r = cur.row, c = cur.col;

        for (int num = 1; num <= 9; num++) {
            if (isValid(board, r, c, num)) {
                auto newBoard = board;
                newBoard[r][c] = num;

                int nr, nc;
                if (!findMRV(newBoard, nr, nc)) {
                    result = newBoard;
                    return true;
                }
                int h = heuristic(newBoard);
                pq.push({newBoard, nr, nc, h});
            }
        }
    }
    return false;
}

// ---------------- 测试 ----------------
vector<vector<int>> sampleInput = {
    {0, 0, 0, 3, 9, 0, 0, 0, 5},
    {9, 0, 0, 0, 0, 0, 0, 4, 1},
    {0, 0, 8, 0, 4, 6, 9, 0, 3},
    {4, 7, 6, 0, 0, 0, 0, 5, 0},
    {0, 0, 2, 6, 0, 3, 4, 0, 0},
    {0, 8, 0, 0, 0, 0, 2, 1, 6},
    {8, 0, 4, 5, 1, 0, 7, 0, 0},
    {2, 1, 0, 0, 0, 0, 0, 0, 4},
    {6, 0, 0, 0, 3, 7, 0, 0, 0}
};

int main() {
    system("chcp 65001 > nul");

    vector<vector<int>> result;
    int expandedBFS = 0, expandedHeuristic = 0;

    cout << "初始数独：" << endl;
    printBoard(sampleInput);
    cout << endl;

    // 普通 BFS
    if (bfsSolve(sampleInput, result, expandedBFS)) {
        cout << "普通 BFS 解：" << endl;
        printBoard(result);
    } else {
        cout << "普通 BFS 无解" << endl;
    }
    cout << "普通 BFS 扩展节点数: " << expandedBFS << endl << endl;

    // 启发式 BFS
    if (heuristicSolve(sampleInput, result, expandedHeuristic)) {
        cout << "启发式 BFS 解：" << endl;
        printBoard(result);
    } else {
        cout << "启发式 BFS 无解" << endl;
    }
    cout << "启发式 BFS 扩展节点数: " << expandedHeuristic << endl;

    return 0;
}
