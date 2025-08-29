#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <climits>
using namespace std;

// ---------- 图 ----------
unordered_map<string, vector<pair<string, int>>> graph = {
    {"Arad", {{"Zerind", 75}, {"Timisoara", 118}, {"Sibiu", 140}}},
    {"Zerind", {{"Arad", 75}, {"Oradea", 71}}},
    {"Oradea", {{"Zerind", 71}, {"Sibiu", 151}}},
    {"Timisoara", {{"Arad", 118}, {"Lugoj", 111}}},
    {"Lugoj", {{"Timisoara", 111}, {"Mehadia", 70}}},
    {"Mehadia", {{"Lugoj", 70}, {"Drobeta", 75}}},
    {"Drobeta", {{"Mehadia", 75}, {"Craiova", 120}}},
    {"Craiova", {{"Drobeta", 120}, {"Rimnicu Vilcea", 146}, {"Pitesti", 138}}},
    {"Sibiu", {{"Arad", 140}, {"Oradea", 151}, {"Fagaras", 99}, {"Rimnicu Vilcea", 80}}},
    {"Fagaras", {{"Sibiu", 99}, {"Bucharest", 211}}},
    {"Rimnicu Vilcea", {{"Sibiu", 80}, {"Craiova", 146}, {"Pitesti", 97}}},
    {"Pitesti", {{"Rimnicu Vilcea", 97}, {"Craiova", 138}, {"Bucharest", 101}}},
    {"Bucharest", {{"Fagaras", 211}, {"Pitesti", 101}, {"Giurgiu", 90}, {"Urziceni", 85}}},
    {"Giurgiu", {{"Bucharest", 90}}},
    {"Urziceni", {{"Bucharest", 85}, {"Vaslui", 142}, {"Hirsova", 98}}},
    {"Hirsova", {{"Urziceni", 98}, {"Eforie", 86}}},
    {"Eforie", {{"Hirsova", 86}}},
    {"Vaslui", {{"Urziceni", 142}, {"Iasi", 92}}},
    {"Iasi", {{"Vaslui", 92}, {"Neamt", 87}}},
    {"Neamt", {{"Iasi", 87}}}};

// ---------- 启发式 h(n) ----------
unordered_map<string, int> heuristic = {
    {"Arad", 366}, {"Bucharest", 0}, {"Craiova", 160}, {"Drobeta", 242}, {"Eforie", 161}, {"Fagaras", 176}, {"Giurgiu", 77}, {"Hirsova", 151}, {"Iasi", 226}, {"Lugoj", 244}, {"Mehadia", 241}, {"Neamt", 234}, {"Oradea", 380}, {"Pitesti", 100}, {"Rimnicu Vilcea", 193}, {"Sibiu", 253}, {"Timisoara", 329}, {"Urziceni", 80}, {"Vaslui", 199}, {"Zerind", 374}};

// ---------- Greedy Best-First Search ----------
void greedyBestFirst(string start, string goal)
{
    // 优先队列比较器：h 小优先
    struct cmp
    {
        bool operator()(pair<int, string> a, pair<int, string> b)
        {
            return a.first > b.first;
        }
    };

    priority_queue<pair<int, string>, vector<pair<int, string>>, cmp> pq;
    unordered_set<string> visited;
    unordered_map<string, string> parent;

    if (!heuristic.count(start) || !heuristic.count(goal))
    {
        cerr << "Heuristic missing key!\n";
        return;
    }

    pq.push(make_pair(heuristic[start], start));

    while (!pq.empty())
    {
        pair<int, string> top = pq.top();
        pq.pop();
        int h = top.first;
        string node = top.second;

        if (visited.count(node))
            continue;
        visited.insert(node);

        cout << "Expand: " << node << " (h=" << heuristic[node] << ")\n";

        if (node == goal)
        {
            vector<string> path;
            string v = goal;
            while (true)
            {
                path.push_back(v);
                if (!parent.count(v))
                    break; // 没有前驱说明到头了
                v = parent[v];
            }
            reverse(path.begin(), path.end());

            cout << "\nReached goal!\nPath: ";
            for (size_t i = 0; i < path.size(); i++)
                cout << path[i] << " ";
            cout << "\n";

            return;
        }

        // 遍历邻居
        for (size_t i = 0; i < graph[node].size(); i++)
        {
            string nbr = graph[node][i].first;
            int edgeCost = graph[node][i].second;
            if (!visited.count(nbr))
            {
                if (!parent.count(nbr))
                    parent[nbr] = node;
                pq.push(make_pair(heuristic[nbr], nbr));
            }
        }
    }
    cout << "No path found!\n";
}

int main()
{
    string start = "Arad";
    string goal = "Bucharest";

    cout << "Greedy Best First Search from " << start << " to " << goal << "\n\n";
    greedyBestFirst(start, goal);

    return 0;
}
