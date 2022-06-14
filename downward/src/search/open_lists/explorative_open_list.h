#ifndef OPEN_LISTS_EXPLORATIVE_OPEN_LIST_H
#define OPEN_LISTS_EXPLORATIVE_OPEN_LIST_H

#include "../open_list_factory.h"
#include "../option_parser_util.h"

/*
  Open list indexed by a single int, using FIFO tie-breaking.

  Implemented as a map from int to deques.
*/

namespace explorative_open_list {
class ExplorativeOpenListFactory : public OpenListFactory {
  Options options;

 public:
  explicit ExplorativeOpenListFactory(const Options &options);
  virtual ~ExplorativeOpenListFactory() override = default;

  virtual std::unique_ptr<StateOpenList> create_state_open_list() override;
  virtual std::unique_ptr<EdgeOpenList> create_edge_open_list() override;
};
}  // namespace exploraive_open_list

#endif
