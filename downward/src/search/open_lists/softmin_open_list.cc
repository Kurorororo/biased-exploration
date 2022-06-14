#include "softmin_open_list.h"

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

namespace softmin_open_list {
template<class Entry>
class SoftminOpenList : public OpenList<Entry> {
    typedef deque<Entry> Bucket;

    shared_ptr<utils::RandomNumberGenerator> rng;
    map<int, Bucket> buckets;
    int size;
    double tau;
    bool ignore_size;
    bool ignore_weights;
    bool relative_h;
    int relative_h_offset;
    double epsilon;
    double current_sum;

    shared_ptr<Evaluator> evaluator;

protected:
    virtual void do_insertion(EvaluationContext &eval_context,
                              const Entry &entry) override;

public:
    explicit SoftminOpenList(const Options &opts);
    SoftminOpenList(const shared_ptr<Evaluator> &eval, bool preferred_only);
    virtual ~SoftminOpenList() override = default;

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
SoftminOpenList<Entry>::SoftminOpenList(const Options &opts)
    : OpenList<Entry>(opts.get<bool>("pref_only")),
      rng(utils::parse_rng_from_options(opts)),
      size(0),
      tau(opts.get<double>("tau")),
      ignore_size(opts.get<bool>("ignore_size")),
      ignore_weights(opts.get<bool>("ignore_weights")),
      relative_h(opts.get<bool>("relative_h")),
      relative_h_offset(opts.get<int>("relative_h_offset")),
      epsilon(opts.get<double>("epsilon")),
      current_sum(0.0),
      evaluator(opts.get<shared_ptr<Evaluator>>("eval")) {
}

template<class Entry>
void SoftminOpenList<Entry>::do_insertion(
    EvaluationContext &eval_context, const Entry &entry) {
    int key = eval_context.get_evaluator_value(evaluator.get());

    if (ignore_size) {
        if (buckets.find(key) == buckets.end()) {
            if (ignore_weights)
                current_sum += 1;
            else if (!relative_h)
                current_sum += std::exp(-1.0 * static_cast<double>(key) / tau);
        }
    } else {
        if (ignore_weights)
            current_sum += 1;
        else if (!relative_h)
            current_sum += std::exp(-1.0 * static_cast<double>(key) / tau);

    }

    buckets[key].push_back(entry);
    ++size;
}

template<class Entry>
Entry SoftminOpenList<Entry>::remove_min() {
    assert(size > 0);
    int key = buckets.begin()->first;

    if (buckets.size() > 1) {
        double r = (*rng)();
        if (r <= epsilon) {
            if (relative_h) {
                double total_sum = 0;
                int i = relative_h_offset;
                for (auto it : buckets) {
                    double s = std::exp(-1.0 * static_cast<double>(i) / tau);
                    if (!ignore_size) s *= static_cast<double>(it.second.size());
                    total_sum += s;
                    ++i;
                }
                double p_sum = 0.0;
                i = relative_h_offset;
                for (auto it : buckets) {
                    double p = std::exp(-1.0 * static_cast<double>(i) / tau) / total_sum;
                    if (!ignore_size) p *= static_cast<double>(it.second.size());
                    p_sum += p;
                    ++i;
                    if (r <= p_sum * epsilon) {
                        key = it.first;
                        break;
                    }
                }
            } else {
                double total_sum = current_sum;
                double p_sum = 0.0;
                for (auto it : buckets) {
                    double p = 1.0 / total_sum;
                    if (!ignore_weights) p *= std::exp(-1.0 * static_cast<double>(it.first) / tau);
                    if (!ignore_size) p *= static_cast<double>(it.second.size());
                    p_sum += p;
                    if (r <= p_sum * epsilon) {
                        key = it.first;
                        break;
                    }
                }
            }
        }
    }

    Bucket &bucket = buckets[key];
    assert(!bucket.empty());
    Entry result = bucket.front();
    bucket.pop_front();
    if (bucket.empty()) {
        buckets.erase(key);
        if (ignore_size) {
            if (ignore_weights)
                current_sum -= 1;
            else if (!relative_h)
                current_sum -= std::exp(-1.0 * static_cast<double>(key) / tau);
        }
    }
    if (!ignore_size) {
        if (ignore_weights)
            current_sum -= 1;
        else if (!relative_h)
            current_sum -= std::exp(-1.0 * static_cast<double>(key) / tau);
    }
    --size;
    return result;
}

template<class Entry>
bool SoftminOpenList<Entry>::empty() const {
    return size == 0;
}

template<class Entry>
void SoftminOpenList<Entry>::clear() {
    buckets.clear();
    size = 0;
}

template<class Entry>
void SoftminOpenList<Entry>::get_path_dependent_evaluators(
    set<Evaluator *> &evals) {
    evaluator->get_path_dependent_evaluators(evals);
}

template<class Entry>
bool SoftminOpenList<Entry>::is_dead_end(
    EvaluationContext &eval_context) const {
    return eval_context.is_evaluator_value_infinite(evaluator.get());
}

template<class Entry>
bool SoftminOpenList<Entry>::is_reliable_dead_end(
    EvaluationContext &eval_context) const {
    return is_dead_end(eval_context) && evaluator->dead_ends_are_reliable();
}

SoftminOpenListFactory::SoftminOpenListFactory(
    const Options &options)
    : options(options) {
}

unique_ptr<StateOpenList>
SoftminOpenListFactory::create_state_open_list() {
    return utils::make_unique_ptr<SoftminOpenList<StateOpenListEntry>>(options);
}

unique_ptr<EdgeOpenList>
SoftminOpenListFactory::create_edge_open_list() {
    return utils::make_unique_ptr<SoftminOpenList<EdgeOpenListEntry>>(options);
}

static shared_ptr<OpenListFactory> _parse(OptionParser &parser) {
    parser.document_synopsis(
        "Softmin open list",
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
    parser.add_option<double>(
        "tau",
        "temperature parameter of softmin", "1.0");
    parser.add_option<bool>(
        "ignore_size",
        "ignore bucket sizes", "false");
    parser.add_option<bool>(
        "ignore_weights",
        "ignore weights of buckets", "false");
    parser.add_option<bool>(
        "relative_h",
        "use relative positions of h-values", "false");
    parser.add_option<int>(
        "relative_h_offset",
        "starting value of relative h-values", "0");
    parser.add_option<double>(
        "epsilon",
        "probability for choosing the next entry randomly",
        "1.0",
        Bounds("0.0", "1.0"));

    utils::add_rng_options(parser);

    Options opts = parser.parse();
    if (parser.dry_run())
        return nullptr;
    else
        return make_shared<SoftminOpenListFactory>(opts);
}

static Plugin<OpenListFactory> _plugin("softmin", _parse);
}


