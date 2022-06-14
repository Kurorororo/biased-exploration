#ifndef OPEN_LISTS_SOFTMIN_TYPE_BASED_OPEN_LIST_H
#define OPEN_LISTS_SOFTMIN_TYPE_BASED_OPEN_LIST_H

#include "../open_list_factory.h"
#include "../option_parser_util.h"


namespace softmin_type_based_open_list {
class SoftminTypeBasedOpenListFactory : public OpenListFactory {
    Options options;
public:
    explicit SoftminTypeBasedOpenListFactory(const Options &options);
    virtual ~SoftminTypeBasedOpenListFactory() override = default;

    virtual std::unique_ptr<StateOpenList> create_state_open_list() override;
    virtual std::unique_ptr<EdgeOpenList> create_edge_open_list() override;
};
}

#endif

