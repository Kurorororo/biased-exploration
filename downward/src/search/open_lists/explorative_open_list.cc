#include "explorative_open_list.h"

#include "../evaluator.h"
#include "../open_list.h"
#include "../option_parser.h"
#include "../plugin.h"

#include "../utils/memory.h"

#include <cassert>
#include <deque>
#include <map>

using namespace std;

namespace explorative_open_list {
template<class Entry>
class ExplorativeOpenList : public OpenList<Entry> {
    typedef deque<Entry> Bucket;

    map<int, Bucket> buckets;
    int size;
    int current_value;
    int n_exploration;
    int max_exploration;

    shared_ptr<Evaluator> evaluator;

protected:
    virtual void do_insertion(EvaluationContext &eval_context,
                              const Entry &entry) override;

public:
    explicit ExplorativeOpenList(const Options &opts);
    ExplorativeOpenList(const shared_ptr<Evaluator> &eval, bool preferred_only);
    virtual ~ExplorativeOpenList() override = default;

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
ExplorativeOpenList<Entry>::ExplorativeOpenList(const Options &opts)
    : OpenList<Entry>(opts.get<bool>("pref_only")),
      size(0),
      current_value(0),
      n_exploration(0),
      max_exploration(opts.get<int>("max_exploration")),
      evaluator(opts.get<shared_ptr<Evaluator>>("eval")) {
}

template<class Entry>
ExplorativeOpenList<Entry>::ExplorativeOpenList(
    const shared_ptr<Evaluator> &evaluator, bool preferred_only)
    : OpenList<Entry>(preferred_only),
      current_value(0),
      n_exploration(0),
      max_exploration(-1),
      size(0),
      evaluator(evaluator) {
}

template<class Entry>
void ExplorativeOpenList<Entry>::do_insertion(
    EvaluationContext &eval_context, const Entry &entry) {
    int key = eval_context.get_evaluator_value(evaluator.get());

    auto it = buckets.begin();
    if (it == buckets.end() || key < it->first) {
        current_value = key;
        n_exploration = 0;
    }

    buckets[key].push_back(entry);
    ++size;
}

template<class Entry>
Entry ExplorativeOpenList<Entry>::remove_min() {
    assert(size > 0);
    auto it = buckets.find(current_value);

    if (it == buckets.end()) {
        it = buckets.begin();
        assert(it != buckets.end());
        current_value = it->first;
        n_exploration = 0;
    }

    assert(it != buckets.end());
    Bucket &bucket = it->second;
    assert(!bucket.empty());
    Entry result = bucket.front();
    bucket.pop_front();

    auto next_it = std::next(it, 1);

    if (bucket.empty())
        buckets.erase(it);
    --size;

    if (next_it == buckets.end() || (max_exploration >= 0 && ++n_exploration > max_exploration)) {
        n_exploration = 0;
        next_it = buckets.begin();
    } else {
        ++n_exploration;
    }
    if (next_it != buckets.end())
        current_value = next_it->first;

    return result;
}

template<class Entry>
bool ExplorativeOpenList<Entry>::empty() const {
    return size == 0;
}

template<class Entry>
void ExplorativeOpenList<Entry>::clear() {
    buckets.clear();
    size = 0;
}

template<class Entry>
void ExplorativeOpenList<Entry>::get_path_dependent_evaluators(
    set<Evaluator *> &evals) {
    evaluator->get_path_dependent_evaluators(evals);
}

template<class Entry>
bool ExplorativeOpenList<Entry>::is_dead_end(
    EvaluationContext &eval_context) const {
    return eval_context.is_evaluator_value_infinite(evaluator.get());
}

template<class Entry>
bool ExplorativeOpenList<Entry>::is_reliable_dead_end(
    EvaluationContext &eval_context) const {
    return is_dead_end(eval_context) && evaluator->dead_ends_are_reliable();
}

ExplorativeOpenListFactory::ExplorativeOpenListFactory(
    const Options &options)
    : options(options) {
}

unique_ptr<StateOpenList>
ExplorativeOpenListFactory::create_state_open_list() {
    return utils::make_unique_ptr<ExplorativeOpenList<StateOpenListEntry>>(options);
}

unique_ptr<EdgeOpenList>
ExplorativeOpenListFactory::create_edge_open_list() {
    return utils::make_unique_ptr<ExplorativeOpenList<EdgeOpenListEntry>>(options);
}

static shared_ptr<OpenListFactory> _parse(OptionParser &parser) {
    parser.document_synopsis(
        "Explorative open list",
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
        "max_exploration",
        "max number of explorations", "-1");

    Options opts = parser.parse();
    if (parser.dry_run())
        return nullptr;
    else
        return make_shared<ExplorativeOpenListFactory>(opts);
}

static Plugin<OpenListFactory> _plugin("explorative", _parse);
}

