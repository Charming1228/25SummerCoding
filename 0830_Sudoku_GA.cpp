#include <iostream>  
#include <vector>    
#include <algorithm> 
#include <random>    // 随机数引擎与分布
#include <chrono>    // 计时与随机种子
using namespace std; 

const int SIZE = 9;     
const int SUB_SIZE = 3; 

// 打印棋盘（每行 9 个数，用空格隔开）
void printBoard(const vector<vector<int>> &board)
{
    for (int i = 0; i < SIZE; ++i)
    { // 遍历每一行
        for (int j = 0; j < SIZE; ++j)
        {                        // 遍历每一列
            cout << board[i][j]; // 输出该格数字
            if (j != SIZE - 1)
                cout << " "; // 非行末添加空格
        }
        cout << endl; // 行末换行
    }
}

// 合法性检查（行/列/宫不能重复）——用于初始化或必要时校验
bool isValid(const vector<vector<int>> &board, int row, int col, int num)
{
    for (int c = 0; c < SIZE; ++c)
    { // 检查所在行
        if (board[row][c] == num)
            return false; // 同行已出现 num
    }
    for (int r = 0; r < SIZE; ++r)
    { // 检查所在列
        if (board[r][col] == num)
            return false; // 同列已出现 num
    }
    int sr = (row / SUB_SIZE) * SUB_SIZE; // 子宫起始行
    int sc = (col / SUB_SIZE) * SUB_SIZE; // 子宫起始列
    for (int i = sr; i < sr + SUB_SIZE; ++i)
    { // 遍历子宫 3 行
        for (int j = sc; j < sc + SUB_SIZE; ++j)
        { // 遍历子宫 3 列
            if (board[i][j] == num)
                return false; // 子宫内已出现 num
        }
    }
    return true; // 三个维度均合法
}

// 找到下一个空格；虽然 GA 不逐格填数，但保留接口
bool findEmpty(const vector<vector<int>> &board, int &row, int &col)
{
    for (row = 0; row < SIZE; ++row)
    { // 行优先扫描
        for (col = 0; col < SIZE; ++col)
        { // 列扫描
            if (board[row][col] == 0)
                return true; // 遇到空格（0）
        }
    }
    return false; // 无空格
}

// GA 专用辅助：固定掩码、初始化、冲突

// 构建固定掩码：题面非 0 的格子标记为固定（GA 过程中不允许改动）
void buildFixedMask(const vector<vector<int>> &given, vector<vector<char>> &fixed)
{
    fixed.assign(SIZE, vector<char>(SIZE, 0)); // 先全部置 0（非固定）
    for (int i = 0; i < SIZE; ++i)
    { // 遍历每行
        for (int j = 0; j < SIZE; ++j)
        { // 遍历每列
            if (given[i][j] != 0)
                fixed[i][j] = 1; // 非 0 即固定
        }
    }
}

// 分宫随机初始化：保证每个 3x3 宫内是一个 1..9 的排列（保持“宫内合法”）
// 这样后续冲突只来自于“行/列”维度，可显著降低搜索难度
void initBoardBlockWise(vector<vector<int>> &b,
                        const vector<vector<int>> &given,
                        const vector<vector<char>> &fixed,
                        mt19937 &rng)
{
    b = given; // 先复制题面
    for (int br = 0; br < SIZE; br += SUB_SIZE)
    { // br：子宫起始行（0,3,6）
        for (int bc = 0; bc < SIZE; bc += SUB_SIZE)
        {                        // bc：子宫起始列（0,3,6）
            bool seen[10] = {0}; // 记录该宫已出现的数字
            for (int i = 0; i < SUB_SIZE; ++i)
            { // 遍历子宫的 3 行
                for (int j = 0; j < SUB_SIZE; ++j)
                {                              // 遍历子宫的 3 列
                    int v = b[br + i][bc + j]; // 当前格的值
                    if (v >= 1 && v <= 9)
                        seen[v] = true; // 已出现标记
                }
            }
            vector<int> missing; // 该宫缺失的数字集合
            for (int d = 1; d <= 9; ++d)
            { // 1..9 遍历
                if (!seen[d])
                    missing.push_back(d); // 没出现则加入缺失
            }
            vector<pair<int, int>> slots; // 该宫内“可填”的非固定空格
            for (int i = 0; i < SUB_SIZE; ++i)
            { // 收集所有可变位置
                for (int j = 0; j < SUB_SIZE; ++j)
                {
                    int r = br + i, c = bc + j; // 绝对坐标
                    if (!fixed[r][c])
                        slots.push_back({r, c}); // 非固定格可填
                }
            }
            shuffle(missing.begin(), missing.end(), rng); // 缺失数字随机打乱
            for (size_t k = 0; k < slots.size(); ++k)
            { // 逐个填入可变位置
                b[slots[k].first][slots[k].second] = missing[k];
            }
            // 注：正常题面会保证 slots.size() == missing.size()
        }
    }
}

// 统计一行的“重复冲突数”：某个数字出现次数>1，则多出的次数记为冲突
int countRowConflicts(const vector<vector<int>> &b, int r)
{
    int cnt[10] = {0}; // 统计 1..9 频次
    int conf = 0;      // 行冲突计数
    for (int c = 0; c < SIZE; ++c)
    {                    // 遍历该行
        int v = b[r][c]; // 当前值
        if (v >= 1 && v <= 9)
            ++cnt[v]; // 计数
    }
    for (int d = 1; d <= 9; ++d)
    { // 遍历 1..9
        if (cnt[d] > 1)
            conf += (cnt[d] - 1); // 多出的都算冲突
    }
    return conf; // 返回本行冲突数
}

// 统计一列的“重复冲突数”（与行同理）
int countColConflicts(const vector<vector<int>> &b, int c)
{
    int cnt[10] = {0}; // 统计 1..9 频次
    int conf = 0;      // 列冲突计数
    for (int r = 0; r < SIZE; ++r)
    {                    // 遍历该列
        int v = b[r][c]; // 当前值
        if (v >= 1 && v <= 9)
            ++cnt[v]; // 计数
    }
    for (int d = 1; d <= 9; ++d)
    { // 遍历 1..9
        if (cnt[d] > 1)
            conf += (cnt[d] - 1); // 多出的都算冲突
    }
    return conf; // 返回本列冲突数
}

// 全局冲突 = 所有行冲突 + 所有列冲突（宫内已保证合法，不需要计入）
int conflicts(const vector<vector<int>> &b)
{
    int s = 0; // 累计器
    for (int r = 0; r < SIZE; ++r)
        s += countRowConflicts(b, r); // 累加行冲突
    for (int c = 0; c < SIZE; ++c)
        s += countColConflicts(b, c); // 累加列冲突
    return s;                         // 返回总冲突
}

// GA 个体与参数 

// 个体：保存一份棋盘与它的冲突代价（cost 越小越好，0 即解）
struct Individual
{
    vector<vector<int>> board; // 个体的棋盘
    int cost;                  // 个体的代价=冲突数
};

// 可调参数：给出合理默认值；你可在 main 里按需修改
struct GAParams
{
    int pop = 200;         // 种群规模
    int generations = 600; // 迭代代数
    int elite = 10;        // 精英保留个数
    double pcross = 0.90;  // 交叉概率
    double pmut = 0.08;    // 变异概率（按“个体”触发）
    unsigned seed = 123;   // 随机种子
};

// 选择 / 交叉 / 变异

// 锦标赛选择：每次随机挑 k 个个体，取其中 cost 最小者的下标
int tournamentSelect(const vector<Individual> &pop, int k, mt19937 &rng)
{
    uniform_int_distribution<int> U(0, (int)pop.size() - 1); // 均匀选下标
    int best = U(rng);                                       // 先随机取一个作为当前最好
    for (int i = 1; i < k; ++i)
    {                   // 再抽 k-1 次
        int t = U(rng); // 新候选下标
        if (pop[t].cost < pop[best].cost)
            best = t; // 更好则更新
    }
    return best; // 返回锦标赛优胜者下标
}

// 分宫交叉：对子代的每个 3x3 宫，随机从父 A 或父 B 复制该宫（保持宫内合法）
// 注意：固定格在父母中相同（等于题面），此处直接复制父母宫即可
Individual crossoverBlockWise(const Individual &A,
                              const Individual &B,
                              const vector<vector<char>> &fixed,
                              mt19937 &rng)
{
    Individual child;      // 创建子代
    child.board = A.board; // 先整体拷贝 A（占位）
    for (int br = 0; br < SIZE; br += SUB_SIZE)
    { // 遍历 3 个子宫起始行
        for (int bc = 0; bc < SIZE; bc += SUB_SIZE)
        {                                                               // 遍历 3 个子宫起始列
            bool takeA = uniform_int_distribution<int>(0, 1)(rng);      // 50% 选 A 或 B
            const vector<vector<int>> &src = takeA ? A.board : B.board; // 选中的父本
            for (int i = 0; i < SUB_SIZE; ++i)
            { // 复制 3 行
                for (int j = 0; j < SUB_SIZE; ++j)
                {                               // 复制 3 列
                    int r = br + i, c = bc + j; // 绝对坐标
                    // 固定位在 A/B 一样（=题面），非固定位从所选父母继承
                    child.board[r][c] = src[r][c]; // 执行复制
                }
            }
        }
    }
    child.cost = -1; // cost 暂未计算
    return child;    // 返回子代
}

// 分宫变异：以 pmut 概率触发；随机选一个 3x3 宫，在其中选两格（非固定）交换
void mutateSwapInRandomBlock(Individual &X,
                             const vector<vector<char>> &fixed,
                             mt19937 &rng,
                             double pmut)
{
    bernoulli_distribution BM(pmut); // 是否触发变异
    if (!BM(rng))
        return;                             // 不触发则返回
    uniform_int_distribution<int> UB(0, 2); // 0..2，用于选子宫行/列块
    int br = UB(rng) * SUB_SIZE;            // 子宫起始行（0/3/6）
    int bc = UB(rng) * SUB_SIZE;            // 子宫起始列（0/3/6）

    // 收集该宫内所有“非固定”的格子（这些格子允许交换）
    struct Cell
    {
        int r, c;
    };                      // 小结构：坐标
    vector<Cell> freeCells; // 存储可交换位置
    for (int i = 0; i < SUB_SIZE; ++i)
    { // 遍历宫内 3 行
        for (int j = 0; j < SUB_SIZE; ++j)
        {                               // 遍历宫内 3 列
            int r = br + i, c = bc + j; // 绝对坐标
            if (!fixed[r][c])
                freeCells.push_back({r, c}); // 非固定才可交换
        }
    }
    if (freeCells.size() < 2)
        return; // 少于 2 个则无法交换

    // 在 freeCells 中随机选两个不同位置做交换
    uniform_int_distribution<int> U(0, (int)freeCells.size() - 1); // 均匀选索引
    int a = U(rng), d = U(rng);                                    // 取两个索引
    if (a == d)
        d = (d + 1) % (int)freeCells.size();    // 确保二者不同
    auto A = freeCells[a], D = freeCells[d];    // 取两格坐标
    swap(X.board[A.r][A.c], X.board[D.r][D.c]); // 执行交换（宫内合法性保持）
}

// GA 主流程

// 返回值：true=找到 0 冲突的解；false=在限定代数内未达 0（仍返回当前最优）
bool gaSolve(const vector<vector<int>> &given,
             vector<vector<int>> &out,
             GAParams P)
{
    // 随机引擎：优先使用外部设定的 seed，默认来自 main
    mt19937 rng(P.seed);

    // 固定掩码：题面非 0 的格子在 GA 中不可改
    vector<vector<char>> fixed;
    buildFixedMask(given, fixed);

    // 初始化种群：每个个体都是“分宫合法”的随机填充
    vector<Individual> pop(P.pop); // 分配 pop 大小的种群空间
    for (int i = 0; i < P.pop; ++i)
    {                                                        // 逐个个体初始化
        initBoardBlockWise(pop[i].board, given, fixed, rng); // 分宫合法随机填充
        pop[i].cost = conflicts(pop[i].board);               // 计算冲突作为 cost
        if (pop[i].cost == 0)
        {                       // 若一开始就碰巧无冲突
            out = pop[i].board; // 输出该解
            return true;        // 直接成功
        }
    }

    // 比较器：用于将种群按 cost 升序排序（cost 越小越优）
    auto cmp = [](const Individual &x, const Individual &y)
    {
        return x.cost < y.cost; // cost 小者在前
    };

    // 交叉概率分布；锦标赛大小选择 3（经验值）
    bernoulli_distribution DoCross(P.pcross); // 是否进行交叉
    const int tourK = 3;                      // 锦标赛选择的 k

    // 为了减少反复 malloc，提前分配 nextPop
    vector<Individual> nextPop(P.pop); // 下一代种群容器

    // 遗传主循环：重复若干代
    for (int gen = 0; gen < P.generations; ++gen)
    {                                      // 迭代 P.generations 代
        sort(pop.begin(), pop.end(), cmp); // 将本代种群按 cost 排序
        if (pop[0].cost == 0)
        {                       // 若最优个体 cost==0
            out = pop[0].board; // 取其棋盘为解
            return true;        // 直接返回成功
        }

        // 精英保留：把前 E 个最优个体原封不动拷到下一代
        int E = min(P.elite, P.pop); // E 不能超过种群规模
        for (int i = 0; i < E; ++i)
        {                        // 拷贝前 E 个
            nextPop[i] = pop[i]; // 直接复制
        }

        // 其余位置由“选择-交叉-变异-评估”产生
        for (int i = E; i < P.pop; ++i)
        {                     // 为下一代的第 i 位产生个体
            Individual child; // 准备子代
            if (DoCross(rng))
            {                                                           // 按概率进行交叉
                int a = tournamentSelect(pop, tourK, rng);              // 选择父 A（锦标赛）
                int b = tournamentSelect(pop, tourK, rng);              // 选择父 B（锦标赛）
                child = crossoverBlockWise(pop[a], pop[b], fixed, rng); // 分宫交叉
            }
            else
            {
                int a = tournamentSelect(pop, tourK, rng); // 不交叉：直接复制一个优秀父代
                child = pop[a];                            // 复制父代为子代
            }
            mutateSwapInRandomBlock(child, fixed, rng, P.pmut); // 以概率进行分宫交换变异
            child.cost = conflicts(child.board);                // 计算子代 cost
            nextPop[i] = child;                                 // 放入下一代
        }

        pop.swap(nextPop); // 进入下一代
    }

    // 代数用尽：返回当前最优个体（可能 cost 非 0），调用者可决定是否继续尝试
    sort(pop.begin(), pop.end(), cmp); // 最后再排序取最佳
    out = pop[0].board;                // 输出最优棋盘
    return (pop[0].cost == 0);         // 若恰好为 0 则 true，否则 false
}

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

int main()
{
    system("chcp 65001 > nul"); 

    // 打印初始盘面
    cout << "初始数独：" << endl; // 标题
    printBoard(sampleInput);      // 输出棋盘
    cout << endl;                 // 空行

    // 设置 GA 参数（可按需调整）
    GAParams P;          // 使用默认值
    P.pop = 200;         // 种群规模
    P.elite = 10;        // 精英保留数
    P.generations = 600; // 最大迭代代数
    P.pcross = 0.90;     // 交叉概率
    P.pmut = 0.08;       // 变异概率
    P.seed = (unsigned)chrono::high_resolution_clock::now()
                 .time_since_epoch()
                 .count(); // 用时间戳作为随机种子

    // 计时并调用 GA 求解
    auto t0 = chrono::high_resolution_clock::now();               // 起始时间
    vector<vector<int>> result;                                   // 保存结果棋盘
    bool ok = gaSolve(sampleInput, result, P);                    // 调用 GA 核心
    auto t1 = chrono::high_resolution_clock::now();               // 结束时间
    double ms = chrono::duration<double, milli>(t1 - t0).count(); // 计算耗时（毫秒）

    // 打印结果
    if (ok)
    {                                                             // 若找到 cost=0 的解
        cout << "求解成功（GA），用时 " << ms << " ms：" << endl; // 成功信息
        printBoard(result);                                       // 打印解
    }
    else
    { // 若未达 0 冲突
        cout << "在设定代数内未达到零冲突（最优冲突="
             << conflicts(result) << "，用时=" << ms << " ms）。" << endl; // 提示调参
        cout << "可提高 generations / pop 或略增 pmut 再试；当前最优棋盘如下：" << endl;
        printBoard(result); // 打印当前最优解（可能仍有冲突）
    }

    return 0; // 正常结束
}
