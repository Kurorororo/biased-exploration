#ifndef OPEN_LISTS_SOFTMIN_HEAP_OPEN_LIST_H
#define OPEN_LISTS_SOFTMIN_HEAP_OPEN_LIST_H

#include "../open_list_factory.h"
#include "../option_parser_util.h"

namespace softmin_heap_open_list {
class SoftminHeapOpenListFactory : public OpenListFactory {
  Options options;

 public:
  explicit SoftminHeapOpenListFactory(const Options &options);
  virtual ~SoftminHeapOpenListFactory() override = default;

  virtual std::unique_ptr<StateOpenList> create_state_open_list() override;
  virtual std::unique_ptr<EdgeOpenList> create_edge_open_list() override;
};
}  // namespace exploraive_open_list

#endif


