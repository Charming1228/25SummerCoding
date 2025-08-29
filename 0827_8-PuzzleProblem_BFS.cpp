#include <bits/stdc++.h>
using namespace std;

struct Board
{
    string s;
    Board(string t = "1238_4765") : s(t) {}
    int blank_pos() const { return (int)s.find('_'); }
    bool operator==(const Board &o) const { return s == o.s; }
    vector<Board> neighbors() const
    {
        static int dr[4] = {-1, 1, 0, 0}, dc[4] = {0, 0, -1, 1};
        vector<Board> res;
        int z = blank_pos(), r = z / 3, c = z % 3;
        for (int k = 0; k < 4; k++)
        {
            int nr = r + dr[k], nc = c + dc[k];
            if (nr < 0 || nr >= 3 || nc < 0 || nc >= 3)
                continue;
            int nz = nr * 3 + nc;
            Board nb = *this;
            swap(nb.s[z], nb.s[nz]);
            res.push_back(nb);
        }
        return res;
    }
    string to_grid() const
    {
        string out;
        for (int i = 0; i < 9; i++)
        {
            out.push_back(s[i]);
            if (i % 3 != 2)
                out.push_back(' ');
            if (i % 3 == 2 && i != 8)
                out.push_back('\n');
        }
        return out;
    }
};

// 可达性判定
int inversion_count(const string &x)
{
    vector<char> a;
    for (char ch : x)
        if (ch != '_')
            a.push_back(ch);
    int inv = 0;
    for (int i = 0; i < (int)a.size(); i++)
        for (int j = i + 1; j < (int)a.size(); j++)
            if (a[i] > a[j])
                inv++;
    return inv;
}
bool solvable(const Board &s, const Board &g)
{
    return inversion_count(s.s) % 2 == inversion_count(g.s) % 2;
}

// 回溯路径
vector<Board> reconstruct(const Board &goal, unordered_map<string, string> &parent)
{
    vector<Board> seq;
    string cur = goal.s;
    while (!cur.empty())
    {
        seq.push_back(Board(cur));
        if (!parent.count(cur))
            break;
        cur = parent[cur];
    }
    reverse(seq.begin(), seq.end());
    return seq;
}

// BFS
vector<Board> bfs(Board start, Board goal)
{
    if (start == goal)
        return vector<Board>(1, start);
    queue<Board> q;
    unordered_set<string> vis;
    unordered_map<string, string> parent;
    q.push(start);
    vis.insert(start.s);
    while (!q.empty())
    {
        Board u = q.front();
        q.pop();
        for (auto v : u.neighbors())
        {
            if (!vis.count(v.s))
            {
                vis.insert(v.s);
                parent[v.s] = u.s;
                if (v == goal)
                    return reconstruct(v, parent);
                q.push(v);
            }
        }
    }
    return vector<Board>();
}

// 打印路径
void print_path(const vector<Board> &path)
{
    for (int i = 0; i < (int)path.size(); i++)
    {
        cout << "Step " << i << ":\n"
             << path[i].to_grid() << "\n\n";
    }
}

int main()
{
    system("chcp 65001 > nul");

    Board goal("1238_4765"); // 新目标状态

    // 6个初始状态
    vector<Board> states = {
        Board("2831647_5"), // State1
        Board("2831_4765"), // State2
        Board("2_3184765"), // State3
        Board("_12345678"), // State4
        Board("1234_6758"), // State5
        Board("152_34678")  // State6
    };

    for (int i = 0; i < 6; i++)
    {
        cout << "=============================\n";
        cout << "State" << (i + 1) << ":\n"
             << states[i].to_grid() << "\n\n";
        if (!solvable(states[i], goal))
        {
            cout << "该状态与目标奇偶性不同，不可达！\n";
            continue;
        }
        vector<Board> path = bfs(states[i], goal);
        if (path.empty())
            cout << "未找到解\n";
        else
        {
            cout << "BFS 找到解，步数=" << (int)path.size() - 1 << "\n";
            print_path(path);
        }
    }
}
