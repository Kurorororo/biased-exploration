#ifndef OPEN_LISTS_LINEAR_WEIGHTED_TYPE_BASED_OPEN_LIST_H
#define OPEN_LISTS_LINEAR_WEIGHTED_TYPE_BASED_OPEN_LIST_H

#include "../open_list_factory.h"
#include "../option_parser_util.h"


namespace linear_weighted_type_based_open_list {
class LinearWeightedTypeBasedOpenListFactory : public OpenListFactory {
    Options options;
public:
    explicit LinearWeightedTypeBasedOpenListFactory(const Options &options);
    virtual ~LinearWeightedTypeBasedOpenListFactory() override = default;

    virtual std::unique_ptr<StateOpenList> create_state_open_list() override;
    virtual std::unique_ptr<EdgeOpenList> create_edge_open_list() override;
};
}

#endif


