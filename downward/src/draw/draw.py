import argparse
import os

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns


def draw_heatmap(filenames, indices, column, normalize, output):
    data_list = []

    for filename in filenames:
        with open(filename) as f:
            data_list.append(
                pd.read_csv(f, usecols=[column] + indices, sep='\t'))

    data = pd.concat(data_list, ignore_index=True).fillna(-1)

    fig = plt.figure(figsize=(5 * len(indices), 5))

    for i, index in enumerate(indices):
        table = pd.crosstab(
            data[index],
            data[column],
            normalize=normalize,
        )
        ax = fig.add_subplot(1, len(indices), i + 1)
        sns.heatmap(table, square=True, cmap="Blues", ax=ax)
        ax.invert_yaxis()

    fig.savefig(output)
    plt.clf()


def draw_gbfs_report(input_filename, output_filename):
    with open(input_filename) as f:
        data = pd.read_csv(f, usecols=['h', 'h*', 'regret h', 'regret h*', 'path'], sep='\t')

    fig = plt.figure(figsize=(15, 5))
    ax = fig.add_subplot(1, 1, 1)
    sns.lineplot(data=data[['h', 'h*', 'regret h', 'regret h*']], markers=True, ax=ax)
    path_points = data[data['path'] == 1].index,
    y_max = max(data['h*'].max(), data['h'].max(), data['regret h'].max())
    ax.vlines(path_points, 0, y_max, linestyles='dashed', label='nodes on the path')
    ax.legend()
    fig.savefig(output_filename)
    plt.clf()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", "-i", type=str, nargs="+", required=True)
    parser.add_argument("--output", "-o", type=str, required=True)
    parser.add_argument("--indices", "-x", type=str, nargs="+", required=True)
    parser.add_argument("--column", "-c", type=str, default="h*")
    parser.add_argument("--normalize", "-n", default="columns")
    parser.add_argument("--gbfs-report-suffix", "-g", type=str)
    parser.add_argument("--gbfs-report-output-suffix", "-b", type=str)
    args = parser.parse_args()

    draw_heatmap(args.input, args.indices, args.column, args.normalize, args.output)

    if args.gbfs_report_suffix:
        input_dirname = os.path.dirname(args.input[0])
        output_dirname = os.path.dirname(args.output)
        for index in args.indices:
            input_filename = os.path.join(input_dirname, index + args.gbfs_report_suffix)
            output_filename = os.path.join(output_dirname, index + args.gbfs_report_output_suffix)
            if os.path.exists(input_filename):
                draw_gbfs_report(input_filename, output_filename)
