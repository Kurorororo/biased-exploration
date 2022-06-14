#ifndef SEARCH_ENGINES_EAGER_SEARCH_H
#define SEARCH_ENGINES_EAGER_SEARCH_H

#include "../open_list.h"
#include "../search_engine.h"

#include <fstream>
#include <memory>
#include <vector>

class Evaluator;
class PruningMethod;

namespace options {
class OptionParser;
class Options;
}

namespace exhaustive_search {
class ExhaustiveSearch : public SearchEngine {
    const bool reopen_closed_nodes;

    std::unique_ptr<StateOpenList> open_list;
    std::shared_ptr<Evaluator> f_evaluator;

    std::vector<Evaluator *> path_dependent_evaluators;
    std::vector<std::shared_ptr<Evaluator>> preferred_operator_evaluators;
    std::shared_ptr<Evaluator> lazy_evaluator;

    std::shared_ptr<PruningMethod> pruning_method;

    std::string filename_to_dump;
    std::vector<std::string> evaluator_descriptions;
    std::unordered_map<std::string, int> evaluator_description_to_id;
    std::vector<StateID> edge_parent_ids;
    std::vector<StateID> edge_successor_ids;
    std::vector<int> edge_costs;
    std::vector<std::vector<int>> edge_h_values;

    void start_f_value_statistics(EvaluationContext &eval_context);
    void update_f_value_statistics(EvaluationContext &eval_context);
    void reward_progress();
    void save_edge(const State &parent, const State &successor, int cost);
    void save_edge(const State &parent, const State &successor, int cost,
                   const EvaluationContext &eval_context);
    void dump_edges() const;

protected:
    virtual void initialize() override;
    virtual SearchStatus step() override;

public:
    explicit ExhaustiveSearch(const options::Options &opts);
    virtual ~ExhaustiveSearch() = default;

    virtual void print_statistics() const override;

    void dump_search_space() const;
};

extern void add_options_to_parser(options::OptionParser &parser);
}

#endif

