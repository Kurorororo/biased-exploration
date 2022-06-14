#include "robust_open_list.h"

#include "../evaluator.h"
#include "../open_list.h"
#include "../option_parser.h"
#include "../plugin.h"

#include "../utils/memory.h"
#include "../utils/rng.h"
#include "../utils/rng_options.h"

#include <cassert>
#include <cmath>
#include <deque>
#include <map>

using namespace std;

namespace robust_open_list {
template<class Entry>
class RobustOpenList : public OpenList<Entry> {
    typedef deque<Entry> Bucket;

    shared_ptr<utils::RandomNumberGenerator> rng;
    map<int, Bucket> buckets;
    int size;
    int delta;
    int beta;
    bool ignore_size;

    shared_ptr<Evaluator> evaluator;

protected:
    virtual void do_insertion(EvaluationContext &eval_context,
                              const Entry &entry) override;

public:
    explicit RobustOpenList(const Options &opts);
    RobustOpenList(const shared_ptr<Evaluator> &eval, bool preferred_only);
    virtual ~RobustOpenList() override = default;

    virtual Entry remove_min() override;
    virtual bool empty() const override;
    virtual void clear() override;
    virtual void get_path_dependent_evaluators(set<Evaluator *> &evals) override;
    virtual bool is_dead_end(
        EvaluationContext &eval_context) const override;
    virtual bool is_reliable_dead_end(
        EvaluationContext &eval_context) const override;
};


template<class Entry>
RobustOpenList<Entry>::RobustOpenList(const Options &opts)
    : OpenList<Entry>(opts.get<bool>("pref_only")),
      rng(utils::parse_rng_from_options(opts)),
      size(0),
      delta(opts.get<int>("delta")),
      beta(opts.get<int>("beta")),
      ignore_size(opts.get<bool>("ignore_size")),
      evaluator(opts.get<shared_ptr<Evaluator>>("eval")) {
}

template<class Entry>
void RobustOpenList<Entry>::do_insertion(
    EvaluationContext &eval_context, const Entry &entry) {
    int key = eval_context.get_evaluator_value(evaluator.get());
    buckets[key].push_back(entry);
    ++size;
}

template<class Entry>
Entry RobustOpenList<Entry>::remove_min() {
    assert(size > 0);
    int h_min = buckets.begin()->first;
    int key = h_min;

    if (buckets.size() > 1 && h_min > beta) {
        double current_sum = 0.0;
        for (auto it : buckets) {
            if (it.first > h_min + delta) break;
            if (ignore_size)
                current_sum += 1.0;
            else
                current_sum += static_cast<double>(it.second.size());
        }
        double r = (*rng)();
        double p_sum = 0.0;
        for (auto it : buckets) {
            if (it.first > h_min + delta) break;
            if (ignore_size) 
                p_sum += 1.0 / current_sum;
            else
                p_sum += static_cast<double>(it.second.size()) / current_sum;
            if (r <= p_sum) {
                key = it.first;
                break;
            }
        }
    }

    Bucket &bucket = buckets[key];
    assert(!bucket.empty());
    Entry result = bucket.front();
    bucket.pop_front();
    if (bucket.empty())
        buckets.erase(key);
    --size;
    return result;
}

template<class Entry>
bool RobustOpenList<Entry>::empty() const {
    return size == 0;
}

template<class Entry>
void RobustOpenList<Entry>::clear() {
    buckets.clear();
    size = 0;
}

template<class Entry>
void RobustOpenList<Entry>::get_path_dependent_evaluators(
    set<Evaluator *> &evals) {
    evaluator->get_path_dependent_evaluators(evals);
}

template<class Entry>
bool RobustOpenList<Entry>::is_dead_end(
    EvaluationContext &eval_context) const {
    return eval_context.is_evaluator_value_infinite(evaluator.get());
}

template<class Entry>
bool RobustOpenList<Entry>::is_reliable_dead_end(
    EvaluationContext &eval_context) const {
    return is_dead_end(eval_context) && evaluator->dead_ends_are_reliable();
}

RobustOpenListFactory::RobustOpenListFactory(
    const Options &options)
    : options(options) {
}

unique_ptr<StateOpenList>
RobustOpenListFactory::create_state_open_list() {
    return utils::make_unique_ptr<RobustOpenList<StateOpenListEntry>>(options);
}

unique_ptr<EdgeOpenList>
RobustOpenListFactory::create_edge_open_list() {
    return utils::make_unique_ptr<RobustOpenList<EdgeOpenListEntry>>(options);
}

static shared_ptr<OpenListFactory> _parse(OptionParser &parser) {
    parser.document_synopsis(
        "Robust open list",
        "Open list that uses a single evaluator and FIFO tiebreaking.");
    parser.document_note(
        "Implementation Notes",
        "Elements with the same evaluator value are stored in double-ended "
        "queues, called \"buckets\". The open list stores a map from evaluator "
        "values to buckets. Pushing and popping from a bucket runs in constant "
        "time. Therefore, inserting and removing an entry from the open list "
        "takes time O(log(n)), where n is the number of buckets.");
    parser.add_option<shared_ptr<Evaluator>>("eval", "evaluator");
    parser.add_option<bool>(
        "pref_only",
        "insert only nodes generated by preferred operators", "false");
    parser.add_option<int>(
        "delta",
        "parameter", "2");
    parser.add_option<int>(
        "beta",
        "parameter", "5");
    parser.add_option<bool>(
        "ignore_size",
        "ignore bucket sizes", "false");

    utils::add_rng_options(parser);

    Options opts = parser.parse();
    if (parser.dry_run())
        return nullptr;
    else
        return make_shared<RobustOpenListFactory>(opts);
}

static Plugin<OpenListFactory> _plugin("robust", _parse);
}



