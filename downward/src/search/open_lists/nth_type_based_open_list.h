#ifndef OPEN_LISTS_NTH_TYPE_BASED_OPEN_LIST_H
#define OPEN_LISTS_NTH_TYPE_BASED_OPEN_LIST_H

#include "../open_list_factory.h"
#include "../option_parser_util.h"


namespace nth_type_based_open_list {
class NthTypeBasedOpenListFactory : public OpenListFactory {
    Options options;
public:
    explicit NthTypeBasedOpenListFactory(const Options &options);
    virtual ~NthTypeBasedOpenListFactory() override = default;

    virtual std::unique_ptr<StateOpenList> create_state_open_list() override;
    virtual std::unique_ptr<EdgeOpenList> create_edge_open_list() override;
};
}

#endif


