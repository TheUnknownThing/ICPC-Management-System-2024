#include <algorithm>
#include <bitset>
#include <chrono>
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#pragma GCC optimize(2)

class Timer {
public:
  void start() { start_time = std::chrono::high_resolution_clock::now(); }

  void stop() {
    auto end_time = std::chrono::high_resolution_clock::now();
    totduration += std::chrono::duration_cast<std::chrono::nanoseconds>(
                       end_time - start_time)
                       .count();
  }

  double duration() const {
    return static_cast<double>(totduration) *
           std::chrono::nanoseconds::period::num /
           std::chrono::nanoseconds::period::den;
  }

  void show() const { std::cout << " took " << duration() << " seconds\n"; }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
  long totduration = 0;
};

struct Team_Info {
  std::string name;
  int data_idx;
};

struct Team_Set {
  std::string name;
  int data_idx;
};

struct Submission {
  std::string name;
  int problem_idx;
  int time;
  std::string stat;
};

struct FrozenSubmissionComp {
  bool operator()(const Submission &a, const Submission &b) const {
    if (a.problem_idx != b.problem_idx) {
      return a.problem_idx < b.problem_idx;
    }
    if (a.stat == "Accepted" && b.stat != "Accepted") {
      return false;
    }
    if (a.time != b.time) {
      return a.time < b.time;
    }
    return a.name < b.name;
  }
};

struct Team_Data {
  std::vector<int> team_time;
  std::vector<Submission> team_submission;
  std::multiset<Submission, FrozenSubmissionComp> frozen_submission;
  int penalty_time;
  std::bitset<26> solved;
  std::bitset<26> actual_solved; // not affected by frozen
  int frozen_attempt[26] = {0};
  int wrong_submission[26] = {0};
  bool submission_need_flush = false;
};

std::vector<Team_Data> team_data;
std::unordered_map<std::string, Team_Info> team_map;
std::vector<std::string> team_rank;
bool isStarted = false;
bool isFrozen = false;
int problem_num, duration;
bool need_flush = true;

struct comp {
  bool operator()(const Team_Set &a, const Team_Set &b) const {
    const Team_Data &data_a = team_data[a.data_idx];
    const Team_Data &data_b = team_data[b.data_idx];

    if (data_a.solved.count() != data_b.solved.count()) {
      return data_a.solved.count() > data_b.solved.count();
    }
    if (data_a.penalty_time != data_b.penalty_time) {
      return data_a.penalty_time < data_b.penalty_time;
    }

    for (int i = data_a.team_time.size() - 1; i >= 0; i--) {
      if (data_a.team_time[i] != data_b.team_time[i]) {
        return data_a.team_time[i] < data_b.team_time[i];
      }
    }
    return a.name < b.name;
  }
};

std::set<Team_Set, comp> team_set;

// <---------------------- Begin of Functions ---------------------->

int CalculateProblemTime(int problem_idx,
                         const std::vector<Submission> &submissions) {
  int penalty_time = 0;
  for (const auto &sub : submissions) {
    if (sub.problem_idx == problem_idx) {
      if (sub.stat != "Accepted") {
        penalty_time += 20;
      } else {
        penalty_time += sub.time;
        break;
      }
    }
  }
  return penalty_time;
}

void AddTeam() {
  std::string name;
  std::cin >> name;
  if (team_map.find(name) != team_map.end()) {
    std::cout << "[Error]Add failed: duplicated team name.\n";
  } else {
    team_data.push_back(Team_Data());
    int idx = team_data.size() - 1;
    team_map[name] = {name, idx};
    std::cout << "[Info]Add successfully.\n";
  }
}

void AddTeamTime(const std::string &team_name, int time) {
  auto &teamtime = team_data[team_map[team_name].data_idx].team_time;
  for (int i = teamtime.size() - 1; i >= 0; i--) {
    if (time > teamtime[i]) {
      teamtime.insert(teamtime.begin() + i + 1, time);
      return;
    }
  }
  teamtime.insert(teamtime.begin(), time);
}

void PrintScoreboard() {
  // team_name rank solved_num penalty_time information_of_each_problem
  for (int i = 0; i < team_rank.size(); i++) {
    std::cout << team_rank[i] << " " << i + 1 << " "
              << team_data[team_map[team_rank[i]].data_idx].solved.count()
              << " " << team_data[team_map[team_rank[i]].data_idx].penalty_time
              << " ";

    for (int j = 0; j < problem_num; j++) {
      bool isSolved = team_data[team_map[team_rank[i]].data_idx].solved[j];
      if (team_data[team_map[team_rank[i]].data_idx].wrong_submission[j] == 0 &&
          !isSolved &&
          !team_data[team_map[team_rank[i]].data_idx].frozen_attempt[j] != 0) {
        std::cout << ". ";
        continue;
      }
      if (isSolved) {
        if (team_data[team_map[team_rank[i]].data_idx].wrong_submission[j] ==
            0) {
          std::cout << "+ ";
        } else {
          std::cout
              << "+"
              << team_data[team_map[team_rank[i]].data_idx].wrong_submission[j]
              << " ";
        }
      } else {
        if (isFrozen) {
          if (team_data[team_map[team_rank[i]].data_idx].wrong_submission[j] ==
              0) {
            std::cout
                << "0/"
                << team_data[team_map[team_rank[i]].data_idx].frozen_attempt[j]
                << " ";
          } else if (team_data[team_map[team_rank[i]].data_idx]
                         .frozen_attempt[j] == 0) {
            std::cout << "-"
                      << team_data[team_map[team_rank[i]].data_idx]
                             .wrong_submission[j]
                      << " ";
          } else {
            std::cout
                << "-"
                << team_data[team_map[team_rank[i]].data_idx]
                       .wrong_submission[j]
                << "/"
                << team_data[team_map[team_rank[i]].data_idx].frozen_attempt[j]
                << " ";
          }
        } else {
          if (isSolved) {
            std::cout << "+"
                      << team_data[team_map[team_rank[i]].data_idx]
                             .wrong_submission[j]
                      << " ";
          } else {
            if (team_data[team_map[team_rank[i]].data_idx]
                    .wrong_submission[j] == 0) {
              std::cout << "0 ";
            } else {
              std::cout << "-"
                        << team_data[team_map[team_rank[i]].data_idx]
                               .wrong_submission[j]
                        << " ";
            }
          }
        }
      }
    }
    std::cout << "\n";
  }
}

void Flush() {
  if (!need_flush) {
    return;
  }
  
  team_set.clear();
  for (const auto &team : team_map) {
    team_set.insert({team.first, team.second.data_idx});
  }
  
  /*if (team_set.empty()) {
    for (const auto &team : team_map) {
      team_set.insert({team.first, team.second.data_idx});
    }
  } else {
    for (const auto &team : team_map) {
      if (team_data[team.second.data_idx].submission_need_flush) {
        team_set.erase({team.first, team.second.data_idx});
        team_set.insert({team.first, team.second.data_idx});
        team_data[team.second.data_idx].submission_need_flush = false;
      }
    }
  }*/

  team_rank.clear();
  for (const auto &team : team_set) {
    team_rank.push_back(team.name);
  }

  need_flush = false;
}

void Submit() {
  std::string team_name, submit_stat, problem_name, temp;
  int time, problem_idx;
  std::cin >> problem_name >> temp >> team_name >> temp >> submit_stat >>
      temp >> time;
  problem_idx = problem_name[0] - 'A';

  auto &team_info = team_map[team_name];
  auto &team = team_data[team_info.data_idx];

  Submission new_submission = {team_name, problem_idx, time, submit_stat};

  team.team_submission.push_back(new_submission);

  if (team.actual_solved[problem_idx]) {
    if (isFrozen) {
      team.frozen_attempt[problem_idx]++;
    }
    return;
  }

  if (submit_stat == "Accepted") {
    team.actual_solved[problem_idx] = true;
  }

  if (isFrozen) {
    team.frozen_submission.insert(new_submission);
    team.frozen_attempt[problem_idx]++;
  } else {
    if (submit_stat == "Accepted") {
      int problem_penalty_time =
          CalculateProblemTime(problem_idx, team.team_submission);
      team.penalty_time += problem_penalty_time;
      AddTeamTime(team_name, time);
      team.solved[problem_idx] = true;
      need_flush = true;
      team.submission_need_flush = true;
    } else {
      team.wrong_submission[problem_idx]++;
    }
  }
}

void Scroll() {
  if (!isFrozen) {
    std::cout << "[Error]Scroll failed: scoreboard has not been frozen.\n";
    return;
  }

  std::cout << "[Info]Scroll scoreboard.\n";

  Flush();
  std::set<Team_Set, comp> temp_set = team_set;

  PrintScoreboard();

  int idx = team_rank.size() - 1;

  while (idx >= 0) {
    std::string current_team_name = team_rank[idx];
    auto &team = team_data[team_map[current_team_name].data_idx];

    if (team.frozen_submission.empty()) {
      idx--;
      continue;
    }

    auto it = team.frozen_submission.begin();
    Submission sub = *it;
    team.frozen_submission.erase(it);

    if (team.solved[sub.problem_idx]) {
      continue;
    }

    if (sub.stat == "Accepted") {
      temp_set.erase({current_team_name, team_map[current_team_name].data_idx});
      int problem_penalty_time =
          CalculateProblemTime(sub.problem_idx, team.team_submission);
      team.penalty_time += problem_penalty_time;
      AddTeamTime(current_team_name, sub.time);
      team.solved[sub.problem_idx] = true;

      std::vector<std::string> prev_team_rank = team_rank;
      team_rank.clear();
      temp_set.insert(
          {current_team_name, team_map[current_team_name].data_idx});

      int new_rank = -1;

      for (const auto &team : temp_set) {
        team_rank.push_back(team.name);
        if (team.name == current_team_name) {
          new_rank = team_rank.size() - 1;
        }
      }

      if (new_rank != idx && new_rank != -1) {
        std::cout << current_team_name << " " << prev_team_rank[new_rank] << " "
                  << team.solved.count() << " " << team.penalty_time << "\n";
      }
    } else {
      team.wrong_submission[sub.problem_idx]++;
    }
  }
  // clear frozen attempt
  for (auto &team : team_data) {
    for (int i = 0; i < problem_num; i++) {
      team.frozen_attempt[i] = 0;
    }
  }
  isFrozen = false;
  PrintScoreboard();
  need_flush = false;
}

void QueryRanking() {
  // QUERY_RANKING [team_name]
  std::string team_name;
  std::cin >> team_name;
  if (team_map.find(team_name) == team_map.end()) {
    std::cout << "[Error]Query ranking failed: cannot find the team.\n";
  } else {
    std::cout << "[Info]Complete query ranking.\n";
    if (isFrozen) {
      std::cout << "[Warning]Scoreboard is frozen. The ranking may be "
                   "inaccurate until it were scrolled.\n";
    }
    int idx =
        find(team_rank.begin(), team_rank.end(), team_name) - team_rank.begin();
    std::cout << team_name << " NOW AT RANKING " << idx + 1 << "\n";
  }
}

void QuerySubmission() {
  std::string team_name;
  std::cin >> team_name;

  if (team_map.find(team_name) == team_map.end()) {
    std::cout << "[Error]Query submission failed: cannot find the team.\n";
    return;
  }

  std::string temp, problem_name, status;
  std::cin >> temp >> problem_name >> temp >> status;
  problem_name = problem_name.substr(8, problem_name.size() - 7);
  status = status.substr(7, status.size() - 7);

  auto &team = team_data[team_map[team_name].data_idx];
  bool found_submission = false;
  std::cout << "[Info]Complete query submission.\n";

  for (auto it = team.team_submission.rbegin();
       it != team.team_submission.rend(); ++it) {
    bool problem_match =
        (problem_name == "ALL" || (problem_name[0] - 'A') == it->problem_idx);
    bool status_match = (status == "ALL" || status == it->stat);

    if (problem_match && status_match) {
      std::cout << team_name << " " << (char)(it->problem_idx + 'A') << " "
                << it->stat << " " << it->time << "\n";
      found_submission = true;
      break;
    }
  }

  if (!found_submission) {
    std::cout << "Cannot find any submission.\n";
  }
}

int main() {
  Timer timer;
  timer.start();

  std::ios::sync_with_stdio(false);
  std::cin.tie(NULL);
  std::cout.tie(NULL);

  freopen("pressure_data/big.in", "r", stdin);
  freopen("output.txt", "w", stdout);

  std::string op;
  while (std::cin >> op) {
    if (op == "END") {
      std::cout << "[Info]Competition ends.\n";
      break;
    } else if (op == "ADDTEAM") {
      if (isStarted) {
        std::cout << "[Error]Add failed: competition has started.\n";
      } else {
        AddTeam();
      }
    } else if (op == "FREEZE") {
      if (isFrozen) {
        std::cout << "[Error]Freeze failed: scoreboard has been frozen.\n";
      } else {
        std::cout << "[Info]Freeze scoreboard.\n";
        isFrozen = true;
      }
    } else if (op == "START") {
      if (isStarted) {
        std::cout << "[Error]Start failed: competition has started.\n";
      } else {
        isStarted = true;
        std::string temp;
        std::cin >> temp >> duration >> temp >> problem_num;
        isStarted = true;
        Flush();
        need_flush = false;
        std::cout << "[Info]Competition starts.\n";
      }
    } else if (op == "SUBMIT") {
      Submit();
    } else if (op == "SCROLL") {
      Scroll();
    } else if (op == "FLUSH") {
      Flush();
      std::cout << "[Info]Flush scoreboard.\n";
    } else if (op == "QUERY_RANKING") {
      QueryRanking();
    } else if (op == "QUERY_SUBMISSION") {
      QuerySubmission();
    }
  }
  timer.stop();
  timer.show();
  return 0;
}