#ifndef OPEN_LISTS_LINEAR_WEIGHTED_HEAP_OPEN_LIST_H
#define OPEN_LISTS_LINEAR_WEIGHTED_HEAP_OPEN_LIST_H

#include "../open_list_factory.h"
#include "../option_parser_util.h"

namespace linear_weighted_heap_open_list {
class LinearWeightedHeapOpenListFactory : public OpenListFactory {
  Options options;

 public:
  explicit LinearWeightedHeapOpenListFactory(const Options &options);
  virtual ~LinearWeightedHeapOpenListFactory() override = default;

  virtual std::unique_ptr<StateOpenList> create_state_open_list() override;
  virtual std::unique_ptr<EdgeOpenList> create_edge_open_list() override;
};
}  // namespace exploraive_open_list

#endif



