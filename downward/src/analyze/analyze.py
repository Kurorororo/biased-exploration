import argparse
import itertools

import pandas as pd
import scipy.stats


def compute_min_ratio(data, heuristic):
    ratio_dict = {}
    for h, d in data.groupby(heuristic):
        ratio_dict[h] = dict(d["h*"].value_counts() / len(d))

    h_values = pd.Series(sorted(data[heuristic].unique()))
    average_r = 0
    average_discordant_ratio = 0
    for h in h_values:
        window = h_values[h_values >= h]
        d_exit = min([min(ratio_dict[h1].keys()) for h1 in window])
        r = 0
        if d_exit in ratio_dict[h]:
            r = ratio_dict[h][d_exit]
        exit_ratio_list = pd.Series([ratio_dict[h1][d_exit] if d_exit in ratio_dict[h1] else 0.0 for h1 in window], index=window)
        average_r += r / exit_ratio_list.max()

        if len(exit_ratio_list) > 1:
            discordant = count_discordant_pairs(exit_ratio_list)
            average_discordant_ratio += discordant / (len(window) * (len(window) - 1) / 2)

    return average_r / len(h_values), average_discordant_ratio / (len(h_values) - 1)


def count_discordant_pairs(a):
    h_values = a.index
    discordant = 0
    for h1, h2 in itertools.combinations(h_values, 2):
        if h1 < h2 and a[h1] >= a[h2]:
            discordant += 1
        if h2 < h1 and a[h2] >= a[h1]:
            discordant += 1

    return discordant


def count_concordant_pairs(a):
    h_values = a.index
    concordant = 0
    for h1, h2 in itertools.combinations(h_values, 2):
        if h1 < h2 and a[h1] <= a[h2]:
            concordant += 1
        if h2 < h1 and a[h2] <= a[h1]:
            concordant += 1
    
    return concordant


def compute_gdrc(df, h_key):
    sorted_df = df.sort_values(["h*", h_key], ignore_index=True)
    h_values = sorted(sorted_df[h_key].unique())
    h_pairs = itertools.combinations(h_values, 2)
    concordant = 0
    discordant = 0

    for pair in h_pairs:
        h1, h2 = pair

        pair_df = sorted_df[sorted_df[h_key].isin(pair)].reset_index(drop=True)
        h1_bin = pair_df[h_key] == h1
        h2_bin = pair_df[h_key] == h2

        h_star_list1 = pair_df[h1_bin]["h*"]
        h_star_list2 = pair_df[h2_bin]["h*"]
        current_ties = 0

        if (
            h_star_list1.max() >= h_star_list2.min()
            or h_star_list1.min() <= h_star_list2.max()
        ):
            count1 = h_star_list1.value_counts()
            count2 = h_star_list2.value_counts()
            current_ties = count1.multiply(count2, fill_value=0).sum()

        h_list = pair_df[h_key]
        counts = h_list.value_counts()
        n_h1 = counts[h1]
        n_h2 = counts[h2]
        h1_bin = h_list == h1
        n_samples = n_h1 * n_h2

        if n_h1 == max(h_list[h1_bin].index) + 1:
            concordant += n_samples - current_ties
            discordant += 0
        else:
            h2_bin = h_list == h2
            h1_cumsum = h1_bin.cumsum()
            h2_int = h2_bin.astype(int)
            concordant_and_ties = (h1_cumsum * h2_int).sum()
            concordant += concordant_and_ties - current_ties
            discordant += n_samples - concordant_and_ties

    total_pairs = len(df) * (len(df) - 1) / 2

    result = {
        "total_pairs": total_pairs,
        "concordant": concordant,
        "discordant": discordant,
        "r_concordant": concordant / total_pairs,
        "r_discordant": discordant / total_pairs,
        "gdrc": (concordant - discordant) / total_pairs
    }

    return result


def evaluate(filename, heuristic):
    data = pd.read_csv(filename, sep="\t", usecols=["h*", heuristic]).dropna(
        subset=[heuristic]
    )
    h_deadend = data["h*"].max() + 1
    data.fillna(h_deadend, inplace=True)

    result = compute_gdrc(data, heuristic)
    result["kendall_tau"], result["kendall_tau_p"] = scipy.stats.kendalltau(data["h*"], data[heuristic])
    result["num_h_values"] = data[heuristic].nunique()
    result["so_mean_gap"], result["so_e_discordant_ratio"] = compute_min_ratio(data, heuristic)

    return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", "-i", type=str, required=True)
    parser.add_argument(
        "--heuristic",
        "-e",
        type=str,
        required=True
    )
    args = parser.parse_args()

    result = evaluate(args.input, args.heuristic)

    print("#h-values: {}".format(result["num_h_values"]))
    print("total_pairs: {}".format(result["total_pairs"]))
    print("concordant: {}".format(result["concordant"]))
    print("discordant: {}".format(result["discordant"]))
    print("r_concordant: {}".format(result["r_concordant"]))
    print("r_discordant: {}".format(result["r_discordant"]))
    print("GDRC: {}".format(result["gdrc"]))
    print("Kendall Tau: {}".format(result["kendall_tau"]))
    print("Mean gap in SO: {}".format(result["so_mean_gap"]))
    print("E[r|h] discordant ratio in SO: {}".format(result["so_e_discordant_ratio"]))