import argparse
import heapq
import json
import math
import random
import statistics

import networkx as nx
import numpy as np
import scipy.stats


def generate_random_instance(n, p, minsize=1, seed=None):
    G = nx.generators.random_graphs.fast_gnp_random_graph(
        n, p, seed=seed, directed=True
    )

    while G.size() < minsize:
        G = nx.generators.random_graphs.fast_gnp_random_graph(
            n, p, seed=seed, directed=True
        )

    n = len(G)
    goal_node = np.random.randint(0, n)

    while G.in_degree(goal_node) == 0:
        goal_node = np.random.randint(0, n)

    dstar_values = dict(nx.single_target_shortest_path_length(G, goal_node))

    nodes = np.array(list(dstar_values.keys()))
    initial_node = np.random.choice(nodes)

    while dstar_values[initial_node] == 0:
        initial_node = np.random.choice(nodes)

    return G, initial_node, goal_node, dstar_values


def local_minima_heuristic(dstar_values, delta):
    values = sorted(list(set(dstar_values.values())))
    h_value_map = {}

    for d in values:
        if d == 0:
            h_value_map[d] = 0
        elif d % (delta + 1) == 1:
            h_value_map[d] = d + delta
        else:
            h_value_map[d] = d - 1

    return {k: h_value_map[d] for k, d in dstar_values.items()}


def gbfs(G, initial_node, goal_node, h_values):
    open_list = []
    generated_set = set()

    heapq.heappush(open_list, (h_values[initial_node], 0, initial_node))
    generated_set.add(initial_node)
    generated = 1
    expanded = 0

    while len(open_list) > 0:
        entry = heapq.heappop(open_list)
        node = entry[2]
        expanded += 1

        if node == goal_node:
            break

        for succ in G.successors(node):
            if succ not in generated_set and succ in h_values:
                h = h_values[succ]
                heapq.heappush(open_list, (h, generated, succ))
                generated_set.add(succ)
                generated += 1

    return expanded


def type_based_push(open_list, t, node):
    if t not in open_list:
        open_list[t] = []

    open_list[t].append(node)


def type_selection(
    open_list, weight=None, tau=1.0, alpha=1.0, beta=0.0, nth=3, delta=None
):
    types = list(open_list.keys())

    if weight == "softmin":
        tmp_weights = [math.exp(-1.0 * t[0] / tau) for t in types]
        weight_sum = sum(tmp_weights)
        weights = [w / weight_sum for w in tmp_weights]

        return random.choices(types, weights=weights)[0]

    if weight == "linear":
        bias = beta + 1.0 + alpha * max(t[0] for t in types)
        tmp_weights = [-1.0 * alpha * t[0] + bias for t in types]
        weight_sum = sum(tmp_weights)
        weights = [w / weight_sum for w in tmp_weights]

        return random.choices(types, weights=weights)[0]

    if weight == "nth":
        hs = sorted(list(set([t[0] for t in types])))[:nth]
        types = [t for t in types if t[0] in hs]

    if weight == "cheat" and delta is not None:
        hs = sorted(list(set([t[0] for t in types])))
        hs = [h for h in hs if hs[0] + delta >= h]
        types = [t for t in types if t[0] in hs]

    return random.choice(types)


def random_pop(bucket):
    i = np.random.randint(0, len(bucket))
    tmp = bucket[i]
    bucket[i] = bucket[-1]
    del bucket[-1]

    return tmp


def type_based_pop(
    open_list, weight=None, tau=1.0, alpha=1.0, beta=0.0, nth=3, delta=None
):
    t = type_selection(
        open_list, weight=weight, tau=tau, alpha=alpha, beta=beta, nth=nth, delta=delta
    )
    entry = random_pop(open_list[t])

    if len(open_list[t]) == 0:
        del open_list[t]

    return entry


def h_type_based_push(open_list, h, t, node):
    if h not in open_list:
        open_list[h] = {}

    if t not in open_list[h]:
        open_list[h][t] = []

    open_list[h][t].append(node)


def h_type_based_pop(
    open_list, weight=None, tau=1.0, alpha=1.0, beta=0.0, nth=3, delta=None
):
    h = type_selection(
        open_list, weight=weight, tau=tau, alpha=alpha, beta=beta, nth=nth, delta=delta
    )
    t = type_selection(open_list[h])

    entry = random_pop(open_list[h][t])

    if len(open_list[h][t]) == 0:
        del open_list[h][t]

        if len(open_list[h]) == 0:
            del open_list[h]

    return entry


def type_gbfs(
    G,
    initial_node,
    goal_node,
    h_values,
    h_type_based=False,
    weight=None,
    tau=1.0,
    alpha=1.0,
    beta=1.0,
    nth=3,
    delta=None,
):
    open_list = []
    closed_list = set()
    type_based_open_list = {}
    g_values = {}

    initial_h = h_values[initial_node]

    heapq.heappush(open_list, (initial_h, 0, initial_node))

    if h_type_based:
        h_type_based_push(
            type_based_open_list, (initial_h,), (initial_h, 0), initial_node
        )
    else:
        type_based_push(type_based_open_list, (initial_h, 0), initial_node)

    g_values[initial_node] = 0

    generated = 1
    expanded = 0
    exploration = False

    while len(open_list) > 0 and len(type_based_open_list) > 0:
        if exploration or len(open_list) == 0:
            if h_type_based:
                node = h_type_based_pop(
                    type_based_open_list,
                    weight=weight,
                    tau=tau,
                    alpha=alpha,
                    beta=beta,
                    nth=nth,
                    delta=delta,
                )
            else:
                node = type_based_pop(
                    type_based_open_list,
                    weight=weight,
                    tau=tau,
                    alpha=alpha,
                    beta=beta,
                    nth=nth,
                    delta=delta,
                )
        else:
            entry = heapq.heappop(open_list)
            node = entry[2]

        if node in closed_list:
            exploration = not exploration
            continue

        closed_list.add(node)
        expanded += 1

        if node == goal_node:
            break

        for succ in G.successors(node):
            if succ not in g_values and succ in h_values:
                h = h_values[succ]
                heapq.heappush(open_list, (h, generated, succ))
                g = g_values[node] + 1

                if h_type_based:
                    h_type_based_push(type_based_open_list, (h,), (h, g), succ)
                else:
                    type_based_push(type_based_open_list, (h, g), succ)

                g_values[succ] = g
                generated += 1

        exploration = not exploration

    return expanded


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--trials", "-t", type=int, default=1000)
    parser.add_argument("--nodes", "-n", type=int, default=10000)
    parser.add_argument("--gamma", "-g", type=float, default=2)
    parser.add_argument("--minsize", "-m", type=float, default=1000)
    parser.add_argument(
        "--delta",
        "-d",
        type=int,
        nargs="+",
        default=[1, 2, 3, 4, 5, 6, 7, 8, 9],
    )
    parser.add_argument("--output", "-o", type=str, required=True)
    parser.add_argument("--seed", "-s", type=int, default=2021)
    args = parser.parse_args()

    random.seed(args.seed)
    np.random.seed(args.seed)

    result = {
        "gbfs": {"min": {}, "median": {}, "max": {}},
        "type": {"min": {}, "median": {}, "max": {}},
        "type-h": {"min": {}, "median": {}, "max": {}},
        "3-type-h": {"min": {}, "median": {}, "max": {}},
        "lin-type-h": {"min": {}, "median": {}, "max": {}},
        "softmin-type-h": {"min": {}, "median": {}, "max": {}},
        "cheating-type-h": {"min": {}, "median": {}, "max": {}},
        "gdrc": {"min": {}, "median": {}, "max": {}},
    }

    for param in args.delta:
        print("param: {}".format(param))
        tmp_result = {key: [] for key in result.keys()}

        for i in range(args.trials):
            G, initial_node, goal_node, dstar_values = generate_random_instance(
                args.nodes,
                args.gamma / (args.nodes - 1),
                minsize=args.minsize,
                seed=args.seed,
            )

            h_values = local_minima_heuristic(dstar_values, param)

            # x = [h_values[k] for k in h_values]
            # y = [dstar_values[k] for k in h_values]
            # sns.scatterplot(x, y)
            # plt.show()

            tmp_result["gbfs"].append(gbfs(G, initial_node, goal_node, h_values))
            tmp_result["type"].append(type_gbfs(G, initial_node, goal_node, h_values))
            tmp_result["type-h"].append(
                type_gbfs(G, initial_node, goal_node, h_values, h_type_based=True)
            )
            tmp_result["3-type-h"].append(
                type_gbfs(
                    G,
                    initial_node,
                    goal_node,
                    h_values,
                    h_type_based=True,
                    weight="nth",
                )
            )
            tmp_result["lin-type-h"].append(
                type_gbfs(
                    G,
                    initial_node,
                    goal_node,
                    h_values,
                    h_type_based=True,
                    weight="linear",
                )
            )
            tmp_result["softmin-type-h"].append(
                type_gbfs(
                    G,
                    initial_node,
                    goal_node,
                    h_values,
                    h_type_based=True,
                    weight="softmin",
                )
            )
            tmp_result["cheating-type-h"].append(
                type_gbfs(
                    G,
                    initial_node,
                    goal_node,
                    h_values,
                    h_type_based=True,
                    weight="cheat",
                    delta=param,
                )
            )

            x = [h_values[node] for node in dstar_values]
            y = [dstar_values[node] for node in dstar_values]
            tau, p = scipy.stats.kendalltau(x, y)
            tmp_result["gdrc"].append(tau)

        for key in result.keys():
            result[key]["median"][param] = statistics.median(tmp_result[key])
            result[key]["max"][param] = max(tmp_result[key])
            result[key]["min"][param] = min(tmp_result[key])

    with open(args.output, "w") as f:
        json.dump(result, f, indent=4)
