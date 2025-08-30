#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <functional>
using namespace std;

#define DEPTH 5        // 最大深度
#define FILE_COUNT "3" // txt文件编号

// 在数组中建立0-5和方位的对应关系
vector<string> direction = {"front", "up", "left", "right", "down", "back"};

struct cube_node
{
    char color[6][3][3]; //6面3*3
    int action_to_me; //从初始节点到当前状态的步数
    vector<string> action; //记录初始状态到当前状态的动作序列
    
    // 重载==运算符用于哈希比较 6*3*3完全一致
    bool operator==(const cube_node& other) const {
        for (int k = 0; k < 6; k++) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (color[k][i][j] != other.color[k][i][j]) {
                        return false;
                    }
                }
            }
        }
        return true;
    }
};

// 哈希函数用于unordered_set 存储 cube_node 避免重复状态
namespace std {
    template<>
    struct hash<cube_node> {
        size_t operator()(const cube_node& node) const {
            size_t hash_val = 0;
            for (int k = 0; k < 6; k++) {
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        hash_val = hash_val * 31 + static_cast<unsigned char>(node.color[k][i][j]);// ①使用31作为乘数 ②避免负数参与运算
                    }
                }
            }
            return hash_val;
        }
    };
}

// 判定该种情况是否为已还原 6面纯色
bool is_target_status(cube_node *this_node)
{
    int k, i, j;
    for (k = 0; k <= 5; k++)
        for (i = 0; i <= 2; i++)
            for (j = 0; j <= 2; j++)
            {
                if (this_node->color[k][i][j] != this_node->color[k][0][0])
                    return false;
            }
    return true;
}

// 对一个面进行顺时针旋转,plane为平面编号《单面自旋顺时针90°》
cube_node *turn_1(cube_node *cur_node, int plane)
{
    cube_node *new_node = new cube_node();
    *new_node = *cur_node;
    int i, j;
    for (i = 0; i <= 2; i++)
        for (j = 0; j <= 2; j++)
            new_node->color[plane][i][2 - j] = cur_node->color[plane][j][i];
            // new[i][2-j] = old[j][i] 第一行变最后一列...
    return new_node;
}

// 对一个面进行逆时针旋转,plane为平面编号
cube_node *turn_0(cube_node *cur_node, int plane)
{
    cube_node *new_node = new cube_node();
    *new_node = *cur_node;
    int i, j;
    for (i = 0; i <= 2; i++)
        for (j = 0; j <= 2; j++)
            new_node->color[plane][2 - i][j] = cur_node->color[plane][j][i];
            // new[2-i][j] = old[j][i] 最后一列变第一行...
    return new_node;
}

// 针对前后上下四个面的顺时针旋转，在动作012中使用
// k表示变换（k=0;1,2）
cube_node *turn012_1(cube_node *cur_node, int k)
{
    cube_node *new_node = new cube_node();
    *new_node = *cur_node;
    int order[4] = {5, 1, 0, 4}; // 5back 1up 0front 4down——>最终回到5back的交换顺序 交换其中一行 后续6个操作实现
    int i, j;
    for (i = 0; i <= 2; i++)
        for (j = 0; j <= 2; j++)
            new_node->color[order[i + 1]][j][k] = cur_node->color[order[i]][j][k];
            // new[order[i+1]][j][k] = old[order[i]][j][k] 按照order顺序交换行
    for (j = 0; j <= 2; j++)
        new_node->color[5][j][k] = cur_node->color[4][j][k];
        // 最后再把down回到back
    return new_node;
}

// 针对前后上下四个面的逆时针旋转，在动作012中使用 把顺时针情况的order倒序即可
cube_node *turn012_0(cube_node *cur_node, int k)
{
    cube_node *new_node = new cube_node();
    *new_node = *cur_node;
    int order[4] = {4, 0, 1, 5}; // 把顺时针情况的order倒序即可
    int i, j;
    for (i = 0; i <= 2; i++)
        for (j = 0; j <= 2; j++)
            new_node->color[order[i + 1]][j][k] = cur_node->color[order[i]][j][k];
    for (j = 0; j <= 2; j++)
        new_node->color[4][j][k] = cur_node->color[5][j][k];
    return new_node;
}

// 0变换的正向（顺时针）
cube_node *set_turn0_1(cube_node *cur_node)
{
    return turn_1(turn012_1(cur_node, 0), 2);
    // 先前后上下四个面的行变换，再对2left面顺时针旋转
}

// 0变换的负向（逆时针）
cube_node *set_turn0_0(cube_node *cur_node)
{
    return turn_0(turn012_0(cur_node, 0), 2);
    // 先前后上下四个面的行变换，再对2left面逆时针旋转
}

// 1变换的正向（顺时针）
cube_node *set_turn1_1(cube_node *cur_node)
{
    return turn012_1(cur_node, 1);
    // 先前后上下四个面的行变换，由于行变换是中间行，不牵扯侧面旋转
}

// 1变换的负向（逆时针）
cube_node *set_turn1_0(cube_node *cur_node)
{
    return turn012_0(cur_node, 1);
    // 先前后上下四个面的行变换，由于行变换是中间行，不牵扯侧面旋转
}

// 2变换的正向（顺时针）
cube_node *set_turn2_1(cube_node *cur_node)
{
    return turn_0(turn012_1(cur_node, 2), 3);
    // 先前后上下四个面的行变换，再对3right面逆时针旋转
}

// 2变换的负向（逆时针）
cube_node *set_turn2_0(cube_node *cur_node)
{
    return turn_1(turn012_0(cur_node, 2), 3);
    // 先前后上下四个面的行变换，再对3right面顺时针旋转
}

// 针对前后左右四个面的顺时针旋转，在动作345中使用
// k是动作对应的数字
cube_node *turn345_1(cube_node *cur_node, int k)
{
    cube_node *new_node = new cube_node();
    *new_node = *cur_node;
    int i;
    int num = k - 3; // num = 2 1 0
    for (i = 0; i <= 2; i++)
    {
        new_node->color[2][i][num] = cur_node->color[0][2 - num][i]; // 前->左
        new_node->color[5][num][2 - i] = cur_node->color[2][i][num]; // 左->后
        new_node->color[3][2 - i][2 - num] = cur_node->color[5][num][2 - i]; // 后->右
        new_node->color[0][2 - num][i] = cur_node->color[3][2 - i][2 - num]; // 右->前
    }
    return new_node;
}

// 针对前后左右四个面的逆时针旋转，在动作345中使用
cube_node *turn345_0(cube_node *cur_node, int k)
{
    cube_node *new_node = new cube_node();
    *new_node = *cur_node;
    int i;
    int num = k - 3; // num = 2 1 0
    for (i = 0; i <= 2; i++)
    {
        new_node->color[0][2 - num][i] = cur_node->color[2][i][num]; // 左->前
        new_node->color[2][i][num] = cur_node->color[5][num][2 - i]; // 后->左
        new_node->color[5][num][2 - i] = cur_node->color[3][2 - i][2 - num]; // 右->后
        new_node->color[3][2 - i][2 - num] = cur_node->color[0][2 - num][i]; // 前->右
    }
    return new_node;
}

// 动作3的正向（顺时针）
cube_node *set_turn3_1(cube_node *cur_node)
{
    return turn_0(turn345_1(cur_node, 3), 4);
}

// 动作3的负向（逆时针）
cube_node *set_turn3_0(cube_node *cur_node)
{
    return turn_1(turn345_0(cur_node, 3), 4);
    // 先前后左右四个面的行变换，再对4down面顺时针旋转
}

// 动作4的正向（顺时针）
cube_node *set_turn4_1(cube_node *cur_node)
{
    return turn345_1(cur_node, 4);
    // 先前后左右四个面的行变换，由于行变换是中间行，不牵扯侧面旋转
}

// 动作4的负向（逆时针）
cube_node *set_turn4_0(cube_node *cur_node)
{
    return turn345_0(cur_node, 4);
    // 先前后左右四个面的行变换，由于行变换是中间行，不牵扯侧面旋转
}

// 动作5的正向（顺时针）
cube_node *set_turn5_1(cube_node *cur_node)
{
    return turn_1(turn345_1(cur_node, 5), 1);
    // 先前后左右四个面的行变换，再对1up面顺时针旋转
}

// 动作5的负向（逆时针）
cube_node *set_turn5_0(cube_node *cur_node)
{
    return turn_0(turn345_0(cur_node, 5), 1);
    // 先前后左右四个面的行变换，再对1up面逆时针旋转
}

// 针对上下左右四个面的顺时针旋转，在动作678中使用
// k是动作对应的数字
cube_node *turn678_1(cube_node *cur_node, int k)
{
    cube_node *new_node = new cube_node();
    *new_node = *cur_node;
    int i, num = 8 - k;
    for (i = 0; i <= 2; i++)
    {
        new_node->color[2][num][i] = cur_node->color[4][2 - num][2 - i]; // 下->左
        new_node->color[1][num][i] = cur_node->color[2][num][i]; // 左->上
        new_node->color[3][num][i] = cur_node->color[1][num][i]; // 上->右
        new_node->color[4][2 - num][2 - i] = cur_node->color[3][num][i]; // 右->下
    }
    return new_node;
}

// 针对上下左右四个面的逆时针旋转，在动作678中使用
// k是动作对应的数字
cube_node *turn678_0(cube_node *cur_node, int k)
{
    cube_node *new_node = new cube_node();
    *new_node = *cur_node;
    int i, num = 8 - k;
    for (i = 0; i <= 2; i++)
    {
        new_node->color[4][2 - num][2 - i] = cur_node->color[2][num][i]; // 左->下
        new_node->color[2][num][i] = cur_node->color[1][num][i]; // 上->左
        new_node->color[1][num][i] = cur_node->color[3][num][i]; // 右->上
        new_node->color[3][num][i] = cur_node->color[4][2 - num][2 - i]; // 下->右
    }
    return new_node;
}

// 动作6的负向（逆时针）
cube_node *set_turn6_0(cube_node *cur_node)
{
    return turn_0(turn678_0(cur_node, 6), 0); 
    // 先上下左右四个面的行变换，再对0front面逆时针旋转
}

// 动作6的正向（顺时针）
cube_node *set_turn6_1(cube_node *cur_node)
{
    return turn_1(turn678_1(cur_node, 6), 0);
    // 先上下左右四个面的行变换，再对0front面顺时针旋转
}

// 动作7的负向（逆时针）
cube_node *set_turn7_0(cube_node *cur_node)
{
    return turn678_0(cur_node, 7);
    // 先上下左右四个面的行变换，由于行变换是中间行，不牵扯侧面旋转
}

// 动作7的正向（顺时针）
cube_node *set_turn7_1(cube_node *cur_node)
{
    return turn678_1(cur_node, 7);
    // 先上下左右四个面的行变换，由于行变换是中间行，不牵扯侧面旋转
}

// 动作8的负向（逆时针）
cube_node *set_turn8_0(cube_node *cur_node)
{
    return turn_1(turn678_0(cur_node, 8), 5);
    // 先上下左右四个面的行变换，再对5back面逆时针旋转
}

// 动作8的正向（顺时针）
cube_node *set_turn8_1(cube_node *cur_node)
{
    return turn_0(turn678_1(cur_node, 8), 5);
    // 先上下左右四个面的行变换，再对5back面顺时针旋转
}

// show:展示当前cube状态
void show(cube_node *this_node)
{
    int i, j, k, m, n, num = this_node->action.size();
    for (k = 0; k <= 5; k++)
    {
        cout << direction[k] << ":" << endl;
        for (i = 0; i <= 2; i++)
        {
            for (j = 0; j <= 2; j++)
                cout << this_node->color[k][i][j] << " ";
            cout << endl;
        }
    }
    return;
}

int main()
{
    cube_node *ori_cube_node = new cube_node();
    ori_cube_node->action_to_me = 0;
    int i, j, k;

    // 读取文件，解析魔方
    string FileName = FILE_COUNT + string(".txt"), line;
    ifstream File(FileName);

    if (!File.is_open())
    {
        cerr << "Error: Cannot open file " << FileName << endl;
        return 1;
    }

    // 直接按照固定顺序读取，不依赖标签
    vector<int> file_to_internal = {5, 4, 0, 2, 3, 1};
    // 按照测试样例的顺序读取 避免映射问题

    for (i = 0; i < 6; i++)
    {
        if (!getline(File, line))
        {
            cerr << "Error: Unexpected end of file at face " << i << endl;
            break;
        }

        line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
        size_t colon_pos = line.find(':');
        if (colon_pos != string::npos)
        {
            line = line.substr(0, colon_pos);
        }

        cout << "Reading face " << i << ": '" << line << "' -> internal index " << file_to_internal[i] << endl;

        int internal_index = file_to_internal[i];

        for (j = 0; j < 3; j++)
        {
            if (!getline(File, line))
            {
                cerr << "Error: Unexpected end of file at row " << j << " of face " << i << endl;
                break;
            }

            line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());

            if (line.length() < 3)
            {
                cerr << "Error: Line too short: '" << line << "'" << endl;
                continue;
            }

            for (k = 0; k < 3; k++)
            {
                ori_cube_node->color[internal_index][j][k] = line[k];
            }
        }
    }

    File.close();

    // 显示读取的魔方状态
    cout << "Loaded cube state:" << endl;
    for (i = 0; i <= 5; i++)
    {
        cout << direction[i] << ":" << endl;
        for (j = 0; j <= 2; j++)
        {
            for (k = 0; k <= 2; k++)
            {
                cout << ori_cube_node->color[i][j][k] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }

    // 检查是否为目标状态
    if (is_target_status(ori_cube_node))
    {
        cout << "Cube is already solved!" << endl;
        delete ori_cube_node;
        return 0;
    }

    // 使用队列进行广度优先搜索
    queue<cube_node*> node_queue;
    unordered_set<cube_node> visited_states;
    vector<cube_node*> all_nodes; // 用于内存管理
    
    node_queue.push(ori_cube_node);
    visited_states.insert(*ori_cube_node); // 防重复状态
    all_nodes.push_back(ori_cube_node); //存new出来的节点最后delete

    bool found = false;
    cube_node *solution_node = nullptr;

    while (!node_queue.empty() && !found)
    {
        cube_node *cur_node = node_queue.front();
        node_queue.pop();

        if (cur_node->action_to_me >= DEPTH) {
            continue;
        }

        // 定义所有可能的旋转操作
        vector<function<cube_node*(cube_node*)>> operations = {
            set_turn0_0, set_turn0_1, set_turn1_0, set_turn1_1,
            set_turn2_0, set_turn2_1, set_turn3_0, set_turn3_1,
            set_turn4_0, set_turn4_1, set_turn5_0, set_turn5_1,
            set_turn6_0, set_turn6_1, set_turn7_0, set_turn7_1,
            set_turn8_0, set_turn8_1
        };

        vector<string> operation_names = {
            "0-", "0+", "1-", "1+", "2-", "2+", "3-", "3+",
            "4-", "4+", "5-", "5+", "6-", "6+", "7-", "7+",
            "8-", "8+"
        };

        for (int op_idx = 0; op_idx < operations.size(); op_idx++)
        {
            cube_node *new_node = operations[op_idx](cur_node);
            
            // 检查是否已经访问过该状态
            if (visited_states.find(*new_node) == visited_states.end())
            {
                new_node->action_to_me = cur_node->action_to_me + 1;
                new_node->action = cur_node->action;
                new_node->action.push_back(operation_names[op_idx]);
                
                if (is_target_status(new_node))
                {
                    solution_node = new_node;
                    found = true;
                    break;
                }
                
                if (new_node->action_to_me < DEPTH)
                {
                    visited_states.insert(*new_node);
                    node_queue.push(new_node);
                    all_nodes.push_back(new_node);
                }
                else
                {
                    delete new_node;
                }
            }
            else
            {
                delete new_node; // 丢弃重复状态
            }
        }
    }

    if (found && solution_node != nullptr)
    {
        cout << "Solution found!" << endl;
        cout << "Steps: " << solution_node->action_to_me << endl;
        cout << "Actions: ";
        for (size_t i = 0; i < solution_node->action.size(); i++)
            cout << solution_node->action[i] << " ";
        cout << endl;
    }
    else
    {
        cout << "No solution found within depth " << DEPTH << endl;
    }

    // 清理内存
    for (auto node : all_nodes)
    {
        delete node;
    }

    return 0;
}