#include "exhaustive_search.h"
#include "search_common.h"

#include "../option_parser.h"
#include "../plugin.h"

using namespace std;

namespace plugin_exhaustive {
static shared_ptr<SearchEngine> _parse(OptionParser &parser) {
    parser.document_synopsis("Exhaustive best-first search", "");

    parser.add_option<shared_ptr<OpenListFactory>>("open", "open list");
    parser.add_option<bool>("reopen_closed",
                            "reopen closed nodes", "false");
    parser.add_option<shared_ptr<Evaluator>>(
        "f_eval",
        "set evaluator for jump statistics. "
        "(Optional; if no evaluator is used, jump statistics will not be displayed.)",
        OptionParser::NONE);
    parser.add_list_option<shared_ptr<Evaluator>>(
        "preferred",
        "use preferred operators of these evaluators", "[]");

    exhaustive_search::add_options_to_parser(parser);
    Options opts = parser.parse();

    shared_ptr<exhaustive_search::ExhaustiveSearch> engine;
    if (!parser.dry_run()) {
        engine = make_shared<exhaustive_search::ExhaustiveSearch>(opts);
    }

    return engine;
}

static Plugin<SearchEngine> _plugin("exhaustive", _parse);
}

