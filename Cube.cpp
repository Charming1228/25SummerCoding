#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <stdio.h>
#include <fstream>
#include <string>
using namespace std;
#define DEPTH 5        // 最大深度
#define FILE_COUNT "1" // txt文件编号
// 代码使用深度搜索，限定搜索深度为5，遍历所有解法，若限定深度为6可在30s左右运行结束

// 旋转变化的函数中，0为逆时针，1为顺时针，如set_turn6_0表示动作6的逆时针
// 0前，1上，2左，3右，4下，5后
// 面色块编号按照展开后从左到右总上到下

// 在数组中建立0-5和方位的对应关系
vector<string> direction = {"front", "up", "left", "right", "down", "back"};

struct cube_node
{
    char color[6][3][3];
    int action_to_me;
    vector<string> action;
};

// 判定该种情况是否为已还原
bool is_target_status(cube_node *this_node)
{
    int k, i, j;
    for (k = 0; k <= 5; k++)
        for (i = 0; i <= 2; i++)
            for (j = 0; j <= 2; j++)
            {
                // 对于一个面来说，如果九个块颜色不全相同，则为false
                if (this_node->color[k][i][j] != this_node->color[k][0][0])
                    return false;
            }
    return true;
}

// 对一个面进行顺时针旋转,plane为平面编号
cube_node *turn_1(cube_node *cur_node, int plane)
{
    cube_node *new_node = new cube_node();
    *new_node = *cur_node;
    int i, j;
    for (i = 0; i <= 2; i++)
        for (j = 0; j <= 2; j++)
            new_node->color[plane][i][2 - j] = cur_node->color[plane][j][i];
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
    return new_node;
}

// 针对前后上下四个面的顺时针旋转，在动作012中使用
// k表示变换（k=0;1,2）
cube_node *turn012_1(cube_node *cur_node, int k)
{
    cube_node *new_node = new cube_node();
    *new_node = *cur_node;
    int order[4] = {5, 1, 0, 4}; // 表示交换顺序，5的某一行给1，1的某一行给0，以此类推
    int i, j;
    for (i = 0; i <= 2; i++)
        for (j = 0; j <= 2; j++)
            new_node->color[order[i + 1]][j][k] = cur_node->color[order[i]][j][k];
    for (j = 0; j <= 2; j++)
        new_node->color[5][j][k] = cur_node->color[4][j][k];
    return new_node;
}

// 针对前后上下四个面的逆时针旋转，在动作012中使用
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
}

// 0变换的负向（逆时针）
cube_node *set_turn0_0(cube_node *cur_node)
{
    return turn_0(turn012_0(cur_node, 0), 2);
}

// 1变换的正向（顺时针）
cube_node *set_turn1_1(cube_node *cur_node)
{
    return turn012_1(cur_node, 1);
}

// 1变换的负向（逆时针）
cube_node *set_turn1_0(cube_node *cur_node)
{
    return turn012_0(cur_node, 1);
}

// 2变换的正向（顺时针）
cube_node *set_turn2_1(cube_node *cur_node)
{
    return turn_0(turn012_1(cur_node, 2), 3);
}

// 2变换的负向（逆时针）
cube_node *set_turn2_0(cube_node *cur_node)
{
    return turn_1(turn012_0(cur_node, 2), 3);
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
        new_node->color[2][i][num] = cur_node->color[0][2 - num][i];
        new_node->color[5][num][2 - i] = cur_node->color[2][i][num];
        new_node->color[3][2 - i][2 - num] = cur_node->color[5][num][2 - i];
        new_node->color[0][2 - num][i] = cur_node->color[3][2 - i][2 - num];
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
        new_node->color[0][2 - num][i] = cur_node->color[2][i][num];
        new_node->color[2][i][num] = cur_node->color[5][num][2 - i];
        new_node->color[5][num][2 - i] = cur_node->color[3][2 - i][2 - num];
        new_node->color[3][2 - i][2 - num] = cur_node->color[0][2 - num][i];
    }
    return new_node;
}

// 动作5的逆时针
cube_node *set_turn5_0(cube_node *cur_node)
{
    return turn_0(turn345_0(cur_node, 5), 1);
}

// 动作5的顺时针
cube_node *set_turn5_1(cube_node *cur_node)
{
    return turn_1(turn345_1(cur_node, 5), 1);
}

// 动作4的逆时针
cube_node *set_turn4_0(cube_node *cur_node)
{
    return turn345_0(cur_node, 4);
}

// 动作4的顺时针
cube_node *set_turn4_1(cube_node *cur_node)
{
    return turn345_1(cur_node, 4);
}

// 动作3的逆时针
cube_node *set_turn3_0(cube_node *cur_node)
{
    return turn_1(turn345_0(cur_node, 3), 4);
}

// 动作3的顺时针
cube_node *set_turn3_1(cube_node *cur_node)
{
    return turn_0(turn345_1(cur_node, 3), 4);
}

// 针对上下左右四个面的顺时针旋转，在动作678中使用
cube_node *turn678_1(cube_node *cur_node, int k)
{
    cube_node *new_node = new cube_node();
    *new_node = *cur_node;
    int i, num = 8 - k;
    for (i = 0; i <= 2; i++)
    {
        new_node->color[2][num][i] = cur_node->color[4][2 - num][2 - i];
        new_node->color[1][num][i] = cur_node->color[2][num][i];
        new_node->color[3][num][i] = cur_node->color[1][num][i];
        new_node->color[4][2 - num][2 - i] = cur_node->color[3][num][i];
    }
    return new_node;
}

// 针对上下左右四个面的逆时针旋转，在动作678中使用
cube_node *turn678_0(cube_node *cur_node, int k)
{
    cube_node *new_node = new cube_node();
    *new_node = *cur_node;
    int i, num = 8 - k;
    for (i = 0; i <= 2; i++)
    {
        new_node->color[4][2 - num][2 - i] = cur_node->color[2][num][i];
        new_node->color[2][num][i] = cur_node->color[1][num][i];
        new_node->color[1][num][i] = cur_node->color[3][num][i];
        new_node->color[3][num][i] = cur_node->color[4][2 - num][2 - i];
    }
    return new_node;
}

// 动作6的逆时针
cube_node *set_turn6_0(cube_node *cur_node)
{
    return turn_0(turn678_0(cur_node, 6), 0);
}

// 动作6的顺时针
cube_node *set_turn6_1(cube_node *cur_node)
{
    return turn_1(turn678_1(cur_node, 6), 0);
}

// 动作7的逆时针
cube_node *set_turn7_0(cube_node *cur_node)
{
    return turn678_0(cur_node, 7);
}

// 动作7的顺时针
cube_node *set_turn7_1(cube_node *cur_node)
{
    return turn678_1(cur_node, 7);
}

// 动作8的逆时针
cube_node *set_turn8_0(cube_node *cur_node)
{
    return turn_1(turn678_0(cur_node, 8), 5);
}

// 动作8的顺时针
cube_node *set_turn8_1(cube_node *cur_node)
{
    return turn_0(turn678_1(cur_node, 8), 5);
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
    // for(i=0;i<=num-1;i++)
    // cout<<this_node->action[i]<<" ";
    // cout<<endl;
    return;
}

int main()
{
    cube_node *ori_cube_node = new cube_node();
    ori_cube_node->action_to_me = 0;
    int i, j, k;

    // 读取文件，解析魔方
    string FileName = to_string(stoi(FILE_COUNT)) + ".txt", line;
    std::ifstream File(FileName);
    char ori_cube1[6][3][3];
    int order[6] = {2, 5, 3, 4, 1, 0};
    for (i = 0; i <= 5; i++)
    {
        getline(File, line);
        for (j = 0; j <= 2; j++)
        {
            getline(File, line);
            for (k = 0; k <= 2; k++)
                ori_cube1[order[i]][j][k] = line[2 * k];
        }
    }

    for (i = 0; i <= 5; i++)
        for (j = 0; j <= 2; j++)
            for (k = 0; k <= 2; k++)
                ori_cube_node->color[i][j][k] = ori_cube1[i][j][k];

    for (i = 0; i <= 5; i++)
    {
        std::cout << "Face " << i << ":\n";
        for (j = 0; j <= 2; j++)
        {
            for (k = 0; k <= 2; k++)
            {
                std::cout << ori_cube_node->color[i][j][k] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    
    vector<cube_node *> old_node_list;
    old_node_list.push_back(ori_cube_node);

    stack<cube_node *> stack0;
    stack0.push(ori_cube_node);

    while (!stack0.empty())
    {
        cube_node *cur_node = stack0.top();
        stack0.pop();

        if (is_target_status(cur_node))
        {
            cout << "Step:" << cur_node->action_to_me << endl;
            int num = cur_node->action.size();
            ;
            for (i = 0; i <= num - 1; i++)
                cout << cur_node->action[i] << " ";
            cout << endl;
            continue;
        }
        else
        {
            if (cur_node->action_to_me >= DEPTH) // 深度为5
                continue;

            cube_node *new_node = set_turn0_0(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("0-");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn0_1(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("0+");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn1_0(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("1-");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn1_1(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("1+");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn2_0(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("2-");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn2_1(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("2+");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn3_0(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("3-");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn3_1(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("3+");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn4_0(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("4-");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn4_1(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("4+");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn5_0(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("5-");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn5_1(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("5+");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn6_0(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("6-");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn6_1(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("6+");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn7_0(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("7-");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn7_1(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("7+");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn8_0(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("8-");
            old_node_list.push_back(new_node);
            stack0.push(new_node);

            new_node = set_turn8_1(cur_node);
            new_node->action_to_me = cur_node->action_to_me + 1;
            new_node->action.push_back("8+");
            old_node_list.push_back(new_node);
            stack0.push(new_node);
        }
    }
    old_node_list.clear();
    return 0;
}
