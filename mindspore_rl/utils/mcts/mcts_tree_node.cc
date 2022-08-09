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

#include <utils/mcts/mcts_tree_node.h>
#include <algorithm>

MonteCarloTreeNodePtr MonteCarloTreeNode::SelectChild() {
  float *selection_value = reinterpret_cast<float *>(AllocateMem(sizeof(float) * children_.size()));
  float *uct_value = reinterpret_cast<float *>(AllocateMem(sizeof(float)));
  // For each child, use selection policy to calculate corresponding value,
  // then choose the largest one.
  int i = 0;
  for (auto &child : children_) {
    bool ret = child->SelectionPolicy(uct_value);
    if (!ret) {
      return nullptr;
    }
    Memcpy(selection_value + i, uct_value, sizeof(float));
    i++;
  }
  int64_t max_position = GetMaxPosition(selection_value, children_.size());
  Free(selection_value);
  Free(uct_value);
  return children_[max_position];
}

MonteCarloTreeNodePtr MonteCarloTreeNode::BestAction() const {
  return *std::max_element(children_.begin(), children_.end(),
                           [](const MonteCarloTreeNodePtr node_a, const MonteCarloTreeNodePtr node_b) {
                             return node_a->BestActionPolicy(node_b);
                           });
}
