#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#pragma GCC optimize(2)

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
  std::set<int> team_time;
  std::vector<Submission> team_submission;
  std::multiset<Submission, FrozenSubmissionComp> frozen_submission;
  int penalty_time;
  std::vector<int> solved;
  std::vector<int> actual_solved; // not affected by frozen
  int frozen_attempt[26] = {0};
  int wrong_submission[26] = {0};
};

std::vector<Team_Data> team_data;
std::unordered_map<std::string, Team_Info> team_map;
std::vector<std::string> team_rank;
bool isStarted = false;
bool isFrozen = false;
int problem_num, duration;

struct comp {
  bool operator()(const Team_Set &a, const Team_Set &b) const {
    const Team_Data &data_a = team_data[a.data_idx];
    const Team_Data &data_b = team_data[b.data_idx];

    if (data_a.solved.size() != data_b.solved.size()) {
      return data_a.solved.size() > data_b.solved.size();
    }
    if (data_a.penalty_time != data_b.penalty_time) {
      return data_a.penalty_time < data_b.penalty_time;
    }
    for (auto it1 = data_a.team_time.end(), it2 = data_b.team_time.end();
         it1 != data_a.team_time.begin() && it2 != data_b.team_time.begin();
         it1--, it2--) {
      if (*it1 != *it2) {
        return *it1 < *it2;
      }
    }
    return a.name < b.name;
  }
};

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

void PrintScoreboard() {
  // team_name rank solved_num penalty_time information_of_each_problem
  for (int i = 0; i < team_rank.size(); i++) {
    std::cout << team_rank[i] << " " << i + 1 << " "
              << team_data[team_map[team_rank[i]].data_idx].solved.size() << " "
              << team_data[team_map[team_rank[i]].data_idx].penalty_time << " ";

    for (int j = 0; j < problem_num; j++) {
      bool isSolved =
          std::find(team_data[team_map[team_rank[i]].data_idx].solved.begin(),
                    team_data[team_map[team_rank[i]].data_idx].solved.end(),
                    j) !=
          team_data[team_map[team_rank[i]].data_idx].solved.end();
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
          if (std::find(
                  team_data[team_map[team_rank[i]].data_idx].solved.begin(),
                  team_data[team_map[team_rank[i]].data_idx].solved.end(), j) !=
              team_data[team_map[team_rank[i]].data_idx].solved.end()) {
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
  std::set<Team_Set, comp> temp_set;
  for (const auto &team_info : team_map) {
    temp_set.insert({team_info.first, team_info.second.data_idx});
  }

  team_rank.clear();
  for (const auto &team : temp_set) {
    team_rank.push_back(team.name);
  }
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

  if (std::find(team.actual_solved.begin(), team.actual_solved.end(),
                problem_idx) != team.actual_solved.end()) {
    if (isFrozen) {
      team.frozen_attempt[problem_idx]++;
    }
    return;
  }

  if (submit_stat == "Accepted") {
    team.actual_solved.push_back(problem_idx);
  }

  if (isFrozen) {
    team.frozen_submission.insert(new_submission);
    team.frozen_attempt[problem_idx]++;
  } else {
    if (submit_stat == "Accepted") {
      int problem_penalty_time =
          CalculateProblemTime(problem_idx, team.team_submission);
      team.penalty_time += problem_penalty_time;
      team.team_time.insert(problem_penalty_time);
      team.solved.push_back(problem_idx);
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
  PrintScoreboard();

  int idx = team_rank.size() - 1;
  while (idx >= 0) {
    auto &team = team_data[team_map[team_rank[idx]].data_idx];

    if (team.frozen_submission.empty()) {
      idx--;
      continue;
    }

    auto it = team.frozen_submission.begin();
    Submission sub = *it;
    team.frozen_submission.erase(it);

    if (std::find(team.solved.begin(), team.solved.end(), sub.problem_idx) !=
        team.solved.end()) {
      continue;
    }

    if (sub.stat == "Accepted") {
      int problem_penalty_time =
          CalculateProblemTime(sub.problem_idx, team.team_submission);
      team.penalty_time += problem_penalty_time;
      team.team_time.insert(problem_penalty_time);
      team.solved.push_back(sub.problem_idx);

      std::vector<std::string> prev_team_rank = team_rank;
      Flush();
      int new_rank =
          find(team_rank.begin(), team_rank.end(), prev_team_rank[idx]) -
          team_rank.begin();

      if (new_rank != idx) {
        std::cout << prev_team_rank[idx] << " " << prev_team_rank[new_rank]
                  << " " << team.solved.size() << " " << team.penalty_time
                  << "\n";
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
  return 0;
}