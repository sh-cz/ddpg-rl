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

#include <utils/mcts/mcts_factory.h>

MonteCarloTreeFactory& MonteCarloTreeFactory::GetInstance() {
  static MonteCarloTreeFactory instance;
  return instance;
}

MonteCarloTreeNodePtr MonteCarloTreeFactory::CreateNode(const std::string& node_name, int action, float prior,
                                                        int player, int64_t tree_handle,
                                                        MonteCarloTreeNodePtr parent_node, int row) {
  auto node_creator = map_node_name_to_node_creator_.find(node_name);
  if (node_creator == map_node_name_to_node_creator_.end()) {
    std::ostringstream oss;
    oss << "Node " << node_name << " not exists.\n";
    oss << "Node register: [";
    for (auto iter = map_node_name_to_node_creator_.begin(); iter != map_node_name_to_node_creator_.end(); iter++) {
      oss << iter->first << " ";
    }
    oss << "]";
    // Return nullptr to catch the exception outside.
    return nullptr;
  }
  auto node = std::shared_ptr<MonteCarloTreeNode>(
      node_creator->second(node_name, action, prior, player, tree_handle, parent_node, row));
  return node;
}

std::tuple<int64_t, MonteCarloTreePtr> MonteCarloTreeFactory::CreateTree(const std::string& tree_name,
                                                                         const std::string& node_name, int player,
                                                                         float max_utility,
                                                                         std::vector<void*> input_global_variable) {
  handle_++;
  auto root = MonteCarloTreeFactory::GetInstance().CreateNode(node_name, 0, 0.0, player, handle_, nullptr, 0);
  if (tree_name == "Common") {
    auto tree = std::make_shared<MonteCarloTree>(root, max_utility, handle_);
    return std::make_tuple(handle_, tree);
  } else {
    auto tree_creator = map_tree_name_to_tree_creator_.find(node_name);
    if (tree_creator == map_tree_name_to_tree_creator_.end()) {
      std::ostringstream oss;
      oss << "Tree " << node_name << " not exists.\n";
      oss << "Tree register: [";
      for (auto iter = map_tree_name_to_tree_creator_.begin(); iter != map_tree_name_to_tree_creator_.end(); iter++) {
        oss << iter->first << " ";
      }
      oss << "]";
      // Return nullptr to catch the exception outside.
      return std::make_tuple(handle_, nullptr);
    }
    auto tree = std::shared_ptr<MonteCarloTree>(tree_creator->second(root, max_utility, handle_));
    map_handle_to_tree_ptr_.insert(std::make_pair(handle_, tree));
    map_handle_to_tree_variable_.insert(std::make_pair(handle_, input_global_variable));
    return std::make_tuple(handle_, tree);
  }
}

void MonteCarloTreeFactory::RegisterNode(const std::string& node_name, NodeCreator&& node_creator) {
  map_node_name_to_node_creator_.insert(std::make_pair(node_name, node_creator));
}

void MonteCarloTreeFactory::RegisterTree(const std::string& tree_name, TreeCreator&& tree_creator) {
  map_tree_name_to_tree_creator_.insert(std::make_pair(tree_name, tree_creator));
}

MonteCarloTreePtr MonteCarloTreeFactory::GetTreeByHandle(int64_t handle) {
  auto iter = map_handle_to_tree_ptr_.find(handle);
  if (iter == map_handle_to_tree_ptr_.end()) {
    std::ostringstream oss;
    oss << "Handle " << handle << " not exists.\n";
    oss << "Handle register: [";
    for (auto pairs = map_handle_to_tree_ptr_.begin(); pairs != map_handle_to_tree_ptr_.end(); pairs++) {
      oss << pairs->first << " ";
    }
    oss << "]";
    // Return nullptr to catch the exception outside.
    return nullptr;
  }
  return iter->second;
}

std::vector<void*> MonteCarloTreeFactory::GetTreeVariableByHandle(int64_t handle) {
  auto iter = map_handle_to_tree_variable_.find(handle);
  if (iter == map_handle_to_tree_variable_.end()) {
    std::ostringstream oss;
    oss << "Handle " << handle << " not exists.\n";
    oss << "Handle register: [";
    for (auto pairs = map_handle_to_tree_variable_.begin(); pairs != map_handle_to_tree_variable_.end(); pairs++) {
      oss << pairs->first << " ";
    }
    oss << "]";
    // Return nullptr to catch the exception outside.
    std::vector<void*> null_vector = {nullptr};
    return null_vector;
  }
  return iter->second;
}