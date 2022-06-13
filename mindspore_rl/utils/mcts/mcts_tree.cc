/**
 * Copyright 2022 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "utils/mcts/mcts_tree.h"
#include <algorithm>
#include "utils/mcts/mcts_factory.h"
#include "utils/mcts/mcts_tree_node.h"

std::vector<int> MonteCarloTree::Selection(int max_length_action) {
  visited_path_.clear();
  visited_path_.emplace_back(root_);
  MonteCarloTreeNodePtr current_node = root_;
  // Create a max length action to avoid dynamic shape
  std::vector<int> action_list(max_length_action, -1);
  int i = 0;
  while (!current_node->IsLeafNode()) {
    auto selected_child = current_node->SelectChild();
    action_list[i] = selected_child->action();
    visited_path_.emplace_back(selected_child);
    current_node = selected_child;
  }
  placeholder_handle_++;
  return action_list;
}

bool MonteCarloTree::Expansion(std::string node_name, int* action, float* prior, int num_action, int player) {
  // Expand the last node of visited_path.
  auto leaf_node = visited_path_.at(visited_path_.size() - 1);
  for (int i = 0; i < num_action; i++) {
    auto action_i = action[i];
    auto prior_i = prior[i];
    auto child_node = MonteCarloTreeFactory::GetInstance().CreateNode(node_name, action_i, prior_i, player,
                                                                      tree_handle_, leaf_node, leaf_node->row());
    leaf_node->AddChild(child_node);
  }
  return true;
}

bool MonteCarloTree::Backpropagation(float* returns) {
  // Reverse the visited path, update from the bottom to the top.
  std::reverse(visited_path_.begin(), visited_path_.end());
  auto leaf_node = visited_path_[0];
  bool solved = false;
  // If the leaf node is terminal, which means that this branch is solved.
  if (leaf_node->terminal()) {
    solved = true;
  }
  // For each node in visited path, call the Update() to update the value.
  // If current branch is solved, backprop the best outcome from the bottom to top.
  for (auto& node : visited_path_) {
    node->Update(returns);
    if (solved && !node->IsLeafNode()) {
      MonteCarloTreeNodePtr best = nullptr;
      bool all_solved = true;
      for (const auto& child : node->children()) {
        if (child->outcome().empty()) {
          all_solved = false;
        } else if (best == nullptr || child->outcome()[child->player()] > best->outcome()[best->player()]) {
          best = child;
        }
      }
      if (best != nullptr && (all_solved || best->outcome()[best->player()] == max_utility_)) {
        node->set_outcome(best->outcome());
      } else {
        solved = false;
      }
    }
  }
  return true;
}
