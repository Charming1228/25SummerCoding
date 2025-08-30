#include <chrono>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <queue>
#include <memory>

using namespace std;

// 常量
constexpr int MAXN = 40;
constexpr int MAXM = 30;
constexpr int MAX_TICKS = 256;
constexpr int MYID = 2023200211; // 替换为你的学号

// 方向定义
constexpr int UP = 0;
constexpr int DOWN = 1;
constexpr int LEFT = 2;
constexpr int RIGHT = 3;
constexpr int STAY = 4; // 用于激活护盾时的原地停留

struct Point {
  int y, x;
  
  // 重载比较运算符
  bool operator==(const Point& other) const {
    return y == other.y && x == other.x;
  }
  
  // 计算与另一点的曼哈顿距离
  int manhattan_distance(const Point& other) const {
    return abs(y - other.y) + abs(x - other.x);
  }
  
  // 获取移动后的点
  Point move(int direction) const {
    Point res = *this;
    switch(direction) {
      case UP: res.y--; break;
      case DOWN: res.y++; break;
      case LEFT: res.x--; break;
      case RIGHT: res.x++; break;
    }
    return res;
  }
};

// 为Point结构体实现哈希函数
namespace std {
  template<> struct hash<Point> {
    size_t operator()(const Point& p) const {
      return hash<int>()(p.y) ^ (hash<int>()(p.x) << 1);
    }
  };
}

struct Item {
  Point pos;
  int value;
  int lifetime;
};

struct Snake {
  int id;
  int length;
  int score;
  int direction;
  int shield_cd;
  int shield_time;
  bool has_key;
  vector<Point> body;

  const Point &get_head() const { return body.front(); }
};

struct Chest {
  Point pos;
  int score;
};

struct Key {
  Point pos;
  int holder_id;
  int remaining_time;
};

struct SafeZoneBounds {
  int x_min, y_min, x_max, y_max;
  
  // 检查点是否在安全区内
  bool contains(const Point& p) const {
    return p.x >= x_min && p.x <= x_max && p.y >= y_min && p.y <= y_max;
  }
};

struct GameState {
  int remaining_ticks;
  vector<Item> items;
  vector<Snake> snakes;
  vector<Chest> chests;
  vector<Key> keys;
  SafeZoneBounds current_safe_zone;
  int next_shrink_tick;
  SafeZoneBounds next_safe_zone;
  int final_shrink_tick;
  SafeZoneBounds final_safe_zone;

  int self_idx;
  
  // 获取当前游戏刻
  int get_current_tick() const {
    return MAX_TICKS - remaining_ticks;
  }
  
  // 获取游戏阶段 (0: 早期, 1: 中期, 2: 后期)
  int get_game_phase() const {
    int tick = get_current_tick();
    if (tick <= 80) return 0;
    if (tick <= 200) return 1;
    return 2;
  }

  const Snake &get_self() const { return snakes[self_idx]; }
};

// A*算法节点结构
struct AStarNode {
    Point pos;                  // 坐标
    int g_cost;                 // 从起点到当前节点的实际代价
    int h_cost;                 // 从当前节点到目标的估计代价（启发式）
    AStarNode* parent;          // 父节点，用于回溯路径
    
    // 总代价 = 实际代价 + 估计代价
    int f_cost() const { return g_cost + h_cost; }
    
    AStarNode(Point p, int g, int h, AStarNode* p_parent) 
        : pos(p), g_cost(g), h_cost(h), parent(p_parent) {}
};

// 比较节点总代价的函数（用于优先队列）
struct CompareNode {
    bool operator()(const AStarNode* a, const AStarNode* b) {
        return a->f_cost() > b->f_cost(); // 小顶堆（总代价小的优先）
    }
};

// 读取游戏状态
void read_game_state(GameState &s) {
  cin >> s.remaining_ticks;

  int item_count;
  cin >> item_count;
  s.items.resize(item_count);
  for (int i = 0; i < item_count; ++i) {
    cin >> s.items[i].pos.y >> s.items[i].pos.x >>
        s.items[i].value >> s.items[i].lifetime;
  }

  int snake_count;
  cin >> snake_count;
  s.snakes.resize(snake_count);
  unordered_map<int, int> id2idx;
  id2idx.reserve(snake_count * 2);

  for (int i = 0; i < snake_count; ++i) {
    auto &sn = s.snakes[i];
    cin >> sn.id >> sn.length >> sn.score >> sn.direction >> sn.shield_cd >>
        sn.shield_time;
    sn.body.resize(sn.length);
    for (int j = 0; j < sn.length; ++j) {
      cin >> sn.body[j].y >> sn.body[j].x;
    }
    sn.has_key = false; // 初始化
    if (sn.id == MYID)
      s.self_idx = i;
    id2idx[sn.id] = i;
  }

  int chest_count;
  cin >> chest_count;
  s.chests.resize(chest_count);
  for (int i = 0; i < chest_count; ++i) {
    cin >> s.chests[i].pos.y >> s.chests[i].pos.x >>
        s.chests[i].score;
  }

  int key_count;
  cin >> key_count;
  s.keys.resize(key_count);
  for (int i = 0; i < key_count; ++i) {
    auto& key = s.keys[i];
    cin >> key.pos.y >> key.pos.x >> key.holder_id >> key.remaining_time;
    if (key.holder_id != -1) {
      auto it = id2idx.find(key.holder_id);
      if (it != id2idx.end()) {
        s.snakes[it->second].has_key = true;
      }
    }
  }

  cin >> s.current_safe_zone.x_min >> s.current_safe_zone.y_min >>
      s.current_safe_zone.x_max >> s.current_safe_zone.y_max;
  cin >> s.next_shrink_tick >> s.next_safe_zone.x_min >>
      s.next_safe_zone.y_min >> s.next_safe_zone.x_max >>
      s.next_safe_zone.y_max;
  cin >> s.final_shrink_tick >> s.final_safe_zone.x_min >>
      s.final_safe_zone.y_min >> s.final_safe_zone.x_max >>
      s.final_safe_zone.y_max;
}

// 检查点是否会碰撞墙壁
bool is_wall_collision(const Point& p) {
  return p.x < 0 || p.x >= MAXN || p.y < 0 || p.y >= MAXM;
}

// 检查点是否会碰撞其他蛇（不包括自己）
bool is_snake_collision(const Point& p, const GameState& state, bool ignore_shielded = false) {
  const Snake& self = state.get_self();
  
  for (const auto& snake : state.snakes) {
    if (snake.id == self.id) continue; // 忽略自己
    
    // 如果忽略有护盾的蛇，则跳过
    if (ignore_shielded && snake.shield_time > 0) continue;
    
    // 检查是否与蛇的身体碰撞
    for (const auto& body_part : snake.body) {
      if (p == body_part) return true;
    }
  }
  
  return false;
}

// 检查点是否会碰撞自己的身体（除了尾部，因为尾部会移动）
bool is_self_collision(const Point& p, const Snake& self) {
  // 检查除了最后一节身体外的所有部分
  for (size_t i = 0; i < self.body.size() - 1; ++i) {
    if (p == self.body[i]) return true;
  }
  return false;
}

// 检查点是否会碰撞宝箱（当没有钥匙时）
bool is_chest_collision(const Point& p, const GameState& state, bool has_key) {
  if (has_key) return false;
  
  for (const auto& chest : state.chests) {
    if (p == chest.pos) return true;
  }
  
  return false;
}

// 检查移动方向是否安全
bool is_direction_safe(int direction, const GameState& state, bool consider_shield = false) {
  const Snake& self = state.get_self();
  Point new_head = self.get_head().move(direction);
  
  // 检查墙壁碰撞（最高优先级）
  if (is_wall_collision(new_head)) return false;
  
  // 检查安全区（如果没有护盾保护）
  bool in_safe_zone = state.current_safe_zone.contains(new_head);
  bool has_shield = self.shield_time > 0 || consider_shield;
  
  if (!in_safe_zone && !has_shield) return false;
  
  // 检查是否碰撞其他蛇（如果没有护盾保护）
  if (!has_shield && is_snake_collision(new_head, state)) return false;
  
  // 检查是否碰撞自己
  if (is_self_collision(new_head, self)) return false;
  
  // 检查是否碰撞宝箱（无钥匙时）
  if (is_chest_collision(new_head, state, self.has_key)) return false;
  
  return true;
}

// 获取所有安全的移动方向
vector<int> get_safe_directions(const GameState& state) {
  vector<int> directions;
  
  for (int dir = UP; dir <= RIGHT; ++dir) {
    if (is_direction_safe(dir, state)) {
      directions.push_back(dir);
    }
  }
  
  return directions;
}

// 评估物品的价值（考虑类型、距离和游戏阶段）
double evaluate_item(const Item& item, const Point& head, const GameState& state) {
  int phase = state.get_game_phase();
  double base_value = 0;
  int distance = head.manhattan_distance(item.pos);
  
  // 避免除以零
  if (distance == 0) distance = 1;
  
  // 根据物品类型和游戏阶段计算基础价值
  if (item.value > 0) { // 普通食物
    base_value = item.value;
    
    // 后期高分食物价值更高
    if (phase == 2 && item.value >= 4) {
      base_value *= 1.5;
    }
  }
  else if (item.value == -1) { // 增长豆
    // 早期更重视增长豆
    base_value = (phase == 0) ? 6 : 3;
  }
  else if (item.value == -2) { // 陷阱
    base_value = -20; // 陷阱有很高的负价值
  }
  
  // 考虑剩余生命周期，快消失的物品价值降低
  double lifetime_factor = min(1.0, (double)item.lifetime / 30.0);
  
  // 距离越近价值越高
  return (base_value * lifetime_factor) / distance;
}

// 评估钥匙的价值
double evaluate_key(const Key& key, const Point& head, const GameState& state) {
  // 如果已经持有钥匙或没有宝箱，则钥匙价值低
  if (state.get_self().has_key || state.chests.empty()) {
    return 0;
  }
  
  // 如果钥匙被持有，价值降低
  if (key.holder_id != -1) {
    return 1.0 / (head.manhattan_distance(key.pos) + 5);
  }
  
  // 有宝箱时钥匙价值高
  double chest_value = 0;
  for (const auto& chest : state.chests) {
    chest_value += chest.score;
  }
  
  int distance = head.manhattan_distance(key.pos);
  return (chest_value * 0.3) / (distance + 1);
}

// 评估宝箱的价值
double evaluate_chest(const Chest& chest, const Point& head, const GameState& state) {
  // 没有钥匙时宝箱是危险的
  if (!state.get_self().has_key) {
    return -1000; // 极高的负价值
  }
  
  int distance = head.manhattan_distance(chest.pos);
  return (double)chest.score / (distance + 1);
}

// 判断对手是否可捕杀（满足阶段、对手弱势、自身安全3大条件）
bool is_opponent_huntable(const Snake& opponent, const GameState& state) {
    const Snake& self = state.get_self();
    int current_phase = state.get_game_phase();
    
    // 条件1：仅中后期才允许捕杀
    if (current_phase < 1) return false; // 0=前期，1=中期，2=后期
    
    // 条件2：对手需满足至少1个弱势特征
    bool opponent_weak = false;
    // 弱势1：对手无护盾
    if (opponent.shield_time == 0) opponent_weak = true;
    // 弱势2：对手蛇头在安全区边缘（下一移动可能出界）
    Point opp_next_head = opponent.get_head().move(opponent.direction);
    if (!state.current_safe_zone.contains(opp_next_head)) opponent_weak = true;
    // 弱势3：对手分数高（死亡后掉落食物多）
    if (opponent.score >= 40) opponent_weak = true;
    if (!opponent_weak) return false;
    
    // 条件3：自身安全（有护盾或长度优势）
    bool self_safe = false;
    // 自身有护盾（主动激活或初始护盾）
    if (self.shield_time > 0) self_safe = true;
    // 自身长度优势（比对手长至少2格，对抗中更灵活）
    if (self.length >= opponent.length + 2) self_safe = true;
    if (!self_safe) return false;
    
    // 额外检查：捕杀路径不能有致命风险
    Point self_head = self.get_head();
    int hunt_dir = -1;
    // 计算朝向对手的方向
    if (self_head.x < opponent.get_head().x) hunt_dir = RIGHT;
    else if (self_head.x > opponent.get_head().x) hunt_dir = LEFT;
    else if (self_head.y < opponent.get_head().y) hunt_dir = DOWN;
    else if (self_head.y > opponent.get_head().y) hunt_dir = UP;
    
    // 若有明确朝向对手的方向，且该方向安全，则判定可捕杀
    if (hunt_dir != -1 && is_direction_safe(hunt_dir, state)) {
        return true;
    }
    
    return false;
}

// 评估捕杀对手的价值（收益=对手分数转化的食物价值，成本=距离）
double evaluate_hunt_value(const Snake& opponent, const Point& self_head, const GameState& state) {
    // 收益：对手分数转化的食物总价值（每份最高20分，向上取整）
    int food_total_value = (opponent.score + 19) / 20 * 10; // 简化计算：平均每份10分
    // 成本：自身头部到对手头部的曼哈顿距离
    int distance = self_head.manhattan_distance(opponent.get_head());
    if (distance == 0) distance = 1; // 避免除以零
    
    // 额外加分：对手即将出安全区（捕杀成功率更高，价值×1.5）
    Point opp_next_head = opponent.get_head().move(opponent.direction);
    if (!state.current_safe_zone.contains(opp_next_head)) {
        food_total_value *= 1.5;
    }
    
    // 价值=收益/成本（与资源评估逻辑一致，便于对比）
    return (double)food_total_value / distance;
}

// 找到最佳目标点和它的价值
pair<Point, double> find_best_target(const GameState& state) {
  const Snake& self = state.get_self();
  const Point& head = self.get_head();
  
  double best_value = -1e9;
  Point best_target = head;
  
  // 评估所有资源
  for (const auto& item : state.items) {
    double value = evaluate_item(item, head, state);
    if (value > best_value) {
      best_value = value;
      best_target = item.pos;
    }
  }
  
  for (const auto& key : state.keys) {
    double value = evaluate_key(key, head, state);
    if (value > best_value) {
      best_value = value;
      best_target = key.pos;
    }
  }
  
  if (self.has_key) {
    for (const auto& chest : state.chests) {
      double value = evaluate_chest(chest, head, state);
      if (value > best_value) {
        best_value = value;
        best_target = chest.pos;
      }
    }
  }
  
  // 评估可捕杀对手（仅中后期）
  int current_phase = state.get_game_phase();
  if (current_phase >= 1) {
    for (const auto& opponent : state.snakes) {
      if (opponent.id == self.id) continue;
      
      if (is_opponent_huntable(opponent, state)) {
        double hunt_value = evaluate_hunt_value(opponent, head, state);
        if (hunt_value > best_value) {
          best_value = hunt_value;
          best_target = opponent.get_head(); // 目标设为对手头部
        }
      }
    }
  }
  
  // 保底目标：安全区中心
  if (best_value <= 0) {
    Point center;
    center.x = (state.current_safe_zone.x_min + state.current_safe_zone.x_max) / 2;
    center.y = (state.current_safe_zone.y_min + state.current_safe_zone.y_max) / 2;
    return {center, 0.1};
  }
  
  return {best_target, best_value};
}

// 选择朝向目标的最佳方向
int choose_direction_towards_target(const vector<int>& safe_directions, 
                                   const Point& head, const Point& target,
                                   const GameState& state) {
  if (safe_directions.empty()) {
    return -1;
  }
  
  // 判断目标是否是对手头部
  bool target_is_opponent_head = false;
  for (const auto& snake : state.snakes) {
    if (snake.get_head() == target && snake.id != state.get_self().id) {
      target_is_opponent_head = true;
      break;
    }
  }
  
  vector<pair<int, int>> dir_priority;
  for (int dir : safe_directions) {
    Point new_head = head.move(dir);
    int distance = new_head.manhattan_distance(target);
    
    // 若目标是对手头部，优先选择能"拦截对手移动"的方向
    if (target_is_opponent_head) {
      // 找到目标对手
      const Snake* opponent = nullptr;
      for (const auto& snake : state.snakes) {
        if (snake.get_head() == target) {
          opponent = &snake;
          break;
        }
      }
      if (opponent != nullptr) {
        // 计算对手下一移动的位置
        Point opp_next_head = opponent->get_head().move(opponent->direction);
        // 若新蛇头能到达对手下一位置，提高优先级
        if (new_head == opp_next_head) {
          distance = max(0, distance - 2);
        }
      }
    }
    
    dir_priority.emplace_back(dir, distance);
  }
  
  // 按距离排序，选择最近的方向
  sort(dir_priority.begin(), dir_priority.end(), 
       [](const pair<int, int>& a, const pair<int, int>& b) {
         return a.second < b.second;
       });
       
  return dir_priority[0].first;
}

// 检查是否需要激活护盾
bool should_activate_shield(const GameState& state) {
  const Snake& self = state.get_self();
  
  // 基础条件校验
  if (self.shield_cd > 0 || self.score < 20 || self.shield_time > 0) {
    return false;
  }
  
  // 无安全方向时激活
  vector<int> safe_dirs = get_safe_directions(state);
  if (safe_dirs.empty()) {
    return true;
  }
  
  // 安全区收缩前激活
  int current_tick = state.get_current_tick();
  Point next_head = self.get_head().move(safe_dirs[0]);
  if (current_tick + 1 == state.next_shrink_tick && !state.next_safe_zone.contains(next_head)) {
    return true;
  }
  
  // 捕杀对手时的护盾激活
  int current_phase = state.get_game_phase();
  if (current_phase >= 1) { // 仅中后期
    for (const auto& opponent : state.snakes) {
      if (opponent.id == self.id || opponent.shield_time > 0) continue;
      
      // 若自身即将与对手碰撞且自身无护盾
      Point self_next_head = self.get_head().move(safe_dirs[0]);
      // 情况1：自身下一位置是对手头部（蛇头对撞）
      if (self_next_head == opponent.get_head()) {
        return true;
      }
      // 情况2：自身下一位置是对手身体
      for (const auto& body_part : opponent.body) {
        if (self_next_head == body_part) {
          return true;
        }
      }
    }
  }
  
  return false;
}

// 检查点是否为有效节点（在地图内且非致命障碍）
bool is_valid_node(const Point& p, const GameState& state, const Snake& self) {
    // 1. 不在地图范围内（墙壁）
    if (p.x < 0 || p.x >= MAXN || p.y < 0 || p.y >= MAXM) 
        return false;
    
    // 2. 碰撞自身身体（除尾部）
    for (size_t i = 0; i < self.body.size() - 1; ++i) {
        if (p == self.body[i]) 
            return false;
    }
    
    // 3. 碰撞无钥匙的宝箱
    if (!self.has_key) {
        for (const auto& chest : state.chests) {
            if (p == chest.pos) 
                return false;
        }
    }
    
    // 4. 安全区外且无护盾（允许短期出安全区，只要有护盾）
    if (!state.current_safe_zone.contains(p) && self.shield_time == 0) 
        return false;
    
    // 允许穿过其他蛇的身体（有护盾时）或无护盾但暂时不碰撞
    return true;
}

// 获取节点的邻居（上下左右四个方向）
vector<Point> get_neighbors(const Point& p) {
    return {
        p.move(UP),
        p.move(DOWN),
        p.move(LEFT),
        p.move(RIGHT)
    };
}

// A*算法：计算从起点到目标的最优路径（返回路径的方向序列）
vector<int> a_star_search(const Point& start, const Point& target, const GameState& state) {
    const Snake& self = state.get_self();
    priority_queue<AStarNode*, vector<AStarNode*>, CompareNode> open_list;
    unordered_map<Point, int> closed_list; // 已探索节点（记录最小g_cost）
    
    // 初始化起点
    AStarNode* start_node = new AStarNode(
        start, 
        0, 
        start.manhattan_distance(target), 
        nullptr
    );
    open_list.push(start_node);
    
    // 限制最大搜索步数，避免性能问题
    const int MAX_SEARCH_STEPS = 50;
    int steps = 0;
    
    while (!open_list.empty() && steps < MAX_SEARCH_STEPS) {
        steps++;
        AStarNode* current = open_list.top();
        open_list.pop();
        
        // 到达目标，回溯路径
        if (current->pos == target) {
            vector<Point> path;
            while (current != nullptr) {
                path.push_back(current->pos);
                current = current->parent;
            }
            reverse(path.begin(), path.end());
            
            // 转换路径为方向序列（从起点的下一步开始）
            vector<int> directions;
            for (size_t i = 1; i < path.size(); ++i) {
                Point prev = path[i-1];
                Point curr = path[i];
                if (curr.y < prev.y) directions.push_back(UP);
                else if (curr.y > prev.y) directions.push_back(DOWN);
                else if (curr.x < prev.x) directions.push_back(LEFT);
                else if (curr.x > prev.x) directions.push_back(RIGHT);
            }
            
            // 释放内存
            while (!open_list.empty()) {
                delete open_list.top();
                open_list.pop();
            }
            delete current;
            return directions;
        }
        
        // 如果已探索过且代价更高，则跳过
        auto it = closed_list.find(current->pos);
        if (it != closed_list.end() && current->g_cost >= it->second) {
            delete current;
            continue;
        }
        closed_list[current->pos] = current->g_cost;
        
        // 探索邻居节点
        for (const Point& neighbor_pos : get_neighbors(current->pos)) {
            // 跳过无效节点
            if (!is_valid_node(neighbor_pos, state, self)) 
                continue;
            
            // 计算新的实际代价（每步代价为1）
            int new_g_cost = current->g_cost + 1;
            
            // 检查是否已探索过且代价更低
            if (closed_list.find(neighbor_pos) != closed_list.end() && 
                new_g_cost >= closed_list[neighbor_pos]) {
                continue;
            }
            
            // 创建新节点并加入开放列表
            AStarNode* neighbor_node = new AStarNode(
                neighbor_pos,
                new_g_cost,
                neighbor_pos.manhattan_distance(target),
                current
            );
            open_list.push(neighbor_node);
        }
        
        delete current;
    }
    
    // 未找到路径或达到最大步数（释放内存）
    while (!open_list.empty()) {
        delete open_list.top();
        open_list.pop();
    }
    return {}; // 返回空路径
}

// 判断是否需要使用A*算法（高价值目标被临时阻挡）
bool need_astar(const GameState& state, const Point& target, double target_value) {
    const Snake& self = state.get_self();
    const Point& head = self.get_head();
    
    // 条件1：目标价值足够高（值得绕路）
    if (target_value < 10) return false; // 阈值可调整
    
    // 条件2：直接路径被阻挡（直线距离内有非致命障碍）
    int direct_distance = head.manhattan_distance(target);
    vector<int> safe_dirs = get_safe_directions(state);
    
    // 检查是否有直接朝向目标的安全方向
    bool has_direct_path = false;
    for (int dir : safe_dirs) {
        Point next_head = head.move(dir);
        if (next_head.manhattan_distance(target) < direct_distance) {
            has_direct_path = true;
            break;
        }
    }
    if (has_direct_path) return false; // 有直接路径则无需A*
    
    // 条件3：自身有护盾（可穿过其他蛇的身体绕路）或游戏后期
    if (self.shield_time == 0 && self.score < 20 && state.get_game_phase() < 2) 
        return false;
    
    return true;
}

// 整合A*的最终决策函数
int get_final_decision(const GameState& state) {
    const Snake& self = state.get_self();
    const Point& head = self.get_head();
    
    // 步骤1：检查是否需要激活护盾（优先于移动）
    if (should_activate_shield(state)) {
        return STAY;
    }
    
    // 步骤2：获取安全方向和最佳目标
    vector<int> safe_dirs = get_safe_directions(state);
    if (safe_dirs.empty()) {
        // 无安全方向，随机尝试
        mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
        uniform_int_distribution<int> dist(0, 3);
        return dist(rng);
    }
    
    // 步骤3：确定最佳目标和价值
    auto [best_target, best_value] = find_best_target(state);
    
    // 步骤4：判断是否需要A*算法
    if (need_astar(state, best_target, best_value)) {
        // 使用A*计算路径
        vector<int> astar_path = a_star_search(head, best_target, state);
        if (!astar_path.empty() && 
            find(safe_dirs.begin(), safe_dirs.end(), astar_path[0]) != safe_dirs.end()) {
            // A*找到有效路径，返回第一步方向
            return astar_path[0];
        }
    }
    
    // 步骤5：默认使用贪心方向选择
    return choose_direction_towards_target(safe_dirs, head, best_target, state);
}

int main() {
    GameState current_state;
    read_game_state(current_state);
    
    // 调用整合A*的决策函数
    int decision = get_final_decision(current_state);
    cout << decision << endl;
    
    return 0;
}
