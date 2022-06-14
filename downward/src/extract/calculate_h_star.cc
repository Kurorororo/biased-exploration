#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <stdexcept> 
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

int read_input(
    const std::string &file_name,
    std::vector<std::string> &h_names,
    std::vector<std::vector<std::pair<int, int>>> &adjacent_list,
    std::vector<std::vector<std::pair<int, int>>> &inverse_adjacent_list,
    std::vector<std::vector<int>> &h_values,
    std::vector<bool> &is_goal) {
    std::ifstream input_file(file_name);
    std::unordered_map<std::string, int> label_to_id;
    std::string line;
    std::string field;
    int label = 0;
    int n_line = 1;

    if (getline(input_file, line)) {
        std::istringstream stream(line);
        // parent
        getline(stream, field, '\t');
        // successor
        getline(stream, field, '\t');
        // cost
        getline(stream, field, '\t');
        // is_goal
        getline(stream, field, '\t');

        while (getline(stream, field, '\t'))
            h_names.push_back(field);
    } else {
        throw std::runtime_error("Input file is empty.");
    }
    ++n_line;

    int root_node = -1;
    h_values.resize(h_names.size(), std::vector<int>());

    while (getline(input_file, line)) {
        std::istringstream stream(line);
        int parent = -1;
        int successor = -1;

        // parent
        if (getline(stream, field, '\t') && !field.empty()) {
            if (label_to_id.find(field) == label_to_id.end()) {
                label_to_id[field] = label++;
                adjacent_list.push_back(std::vector<std::pair<int, int>>());
                inverse_adjacent_list.push_back(std::vector<std::pair<int, int>>());
                for (auto &v : h_values)
                    v.push_back(-1);
                is_goal.push_back(false);
            }
            parent = label_to_id[field];
        }

        // successor
        if (getline(stream, field, '\t') && !field.empty()) {
            if (label_to_id.find(field) == label_to_id.end()) {
                label_to_id[field] = label++;
                adjacent_list.push_back(std::vector<std::pair<int, int>>());
                inverse_adjacent_list.push_back(std::vector<std::pair<int, int>>());
                for (auto &v : h_values)
                    v.push_back(-1);
                is_goal.push_back(false);
            }
            successor = label_to_id[field];
        } else {
            std::string message = "Invalid input at line " + std::to_string(n_line) + ": every row must contain the label of the successor node.";
            throw std::runtime_error(message);
        }

        if (parent ==  -1) {
            if (root_node == -1) {
                root_node = successor;
            } else {
                std::string message = "Invalid input at line " + std::to_string(n_line) +": thre are more than one root nodes.";
                throw std::runtime_error(message);
            }
        }

        // cost
        if (getline(stream, field, '\t') && !field.empty() && parent != -1) {
            int cost = -1;
            try {
                cost = std::stoi(field);
            } catch (const std::invalid_argument& e) {
                std::string message = "Invalid input at line " + std::to_string(n_line) + ": costs must be integers.";
                throw std::runtime_error(message);
            }
            adjacent_list[parent].push_back(std::make_pair(successor, cost));
            inverse_adjacent_list[successor].push_back(std::make_pair(parent, cost));
        }

        // is_goal
        if (getline(stream, field, '\t') && !field.empty()) {
            if (std::stoi(field) == 1)
                is_goal[successor] = true;
        } else {
            std::string message = "Invalid input at line " + std::to_string(n_line) + ": is_goal must be 0 or 1.";
            throw std::runtime_error(message);
        }

        // h-values
        for (int i = 0, n = h_names.size(); i < n; ++i) {
            if (getline(stream, field, '\t') && !field.empty()) {
                int h = -1;
                try {
                    h = std::stoi(field);
                } catch (const std::invalid_argument& e) {
                    std::string message = "Invalid input at line " + std::to_string(n_line) + ": h-values must be integers.";
                    throw std::runtime_error(message);
                }
                h_values[i][successor] = h;
            }
        }
        ++n_line;
    }

    if (root_node == -1)
        throw std::runtime_error("Invalid input: there is no root node.");

    return root_node;
}

void write_output(
    const std::string &output_filename,
    const std::vector<std::string> &h_names,
    const std::vector<std::vector<std::pair<int, int>>> &adjacent_list,
    const std::vector<bool> &is_goal,
    const std::vector<std::vector<int>> &h_values,
    const std::vector<int> &h_star_values) {
    std::ofstream output_file(output_filename);
    output_file << "node\tis_goal\th*";
    for (auto name : h_names)
        output_file << "\t" << name;
    output_file << "\tedges\tedge costs" << std::endl;

    for (int node = 0, n_nodes = adjacent_list.size(); node < n_nodes; ++node) {
        output_file << node;
        if (is_goal[node])
            output_file << "\t1";
        else
            output_file << "\t0";
        if (h_star_values[node] == -1)
            output_file << "\t"; 
        else
            output_file << "\t" << h_star_values[node];
        for (int i = 0, n = h_names.size(); i < n; ++i) {
            if (h_values[i][node] != -1)
                output_file << "\t" << h_values[i][node];
            else
                output_file << "\t";
        }
        output_file << "\t";
        for (int i = 0, n = adjacent_list[node].size(); i < n; ++i) {
            output_file << adjacent_list[node][i].first;
            if (i < n - 1) output_file << ",";
        }
        output_file << "\t";
        for (int i = 0, n = adjacent_list[node].size(); i < n; ++i) {
            output_file << adjacent_list[node][i].second;
            if (i < n - 1) output_file << ",";
        }
        output_file << std::endl;
    }
}

void calculate_h_star(
    const std::vector<std::vector<std::pair<int, int>>> &inverse_adjacent_list,
    const std::vector<bool> &is_goal,
    std::vector<int> &h_star_values) {
    h_star_values = std::vector<int>(inverse_adjacent_list.size(), -1);
    std::priority_queue<
        std::pair<int, int>,
        std::vector<std::pair<int, int>>,
        std::greater<std::pair<int, int>> > q;

    for (int i = 0, n = is_goal.size(); i < n; ++i)
        if (is_goal[i])
            q.push(std::make_pair(0, i));

    while (!q.empty()) {
        auto top = q.top();
        q.pop();
        int cost = top.first;
        int node = top.second;

        if (h_star_values[node] == -1 || cost < h_star_values[node]) {
            h_star_values[node] = cost;

            for (auto edge: inverse_adjacent_list[node]) {
                int predecessor_cost = cost + edge.second;
                int predecessor = edge.first;

                if (h_star_values[predecessor] == -1 || predecessor_cost < h_star_values[predecessor])
                    q.push(std::make_pair(predecessor_cost, predecessor));
            }
        }
    }
}

void report_gbfs(
    int root_node,
    const std::vector<std::vector<std::pair<int, int>>> &adjacent_list,
    const std::vector<bool> &is_goal,
    const std::vector<int> &h_values,
    const std::vector<int> &h_star_values,
    std::string output_filename) {
    std::vector<int> node_log;
    std::vector<int> h_log;
    std::vector<int> h_star_log;
    std::vector<int> regret_node_log;
    std::vector<int> regret_h_log;
    std::vector<int> regret_h_star_log;

    std::priority_queue<
        std::pair<int, int>,
        std::vector<std::pair<int, int>>,
        std::greater<std::pair<int, int>> > open;
    // (parent, closed)
    std::unordered_map<int, std::pair<int, bool>> generated;
    std::unordered_set<int> path;
    std::priority_queue<
        std::tuple<int, int, int>,
        std::vector<std::tuple<int, int, int>>,
        std::greater<std::tuple<int, int, int>> > h_star_queue;

    generated[root_node] = std::make_pair(-1, false);
    if (h_values[root_node] != -1) {
        open.push(std::make_pair(h_values[root_node], root_node));
        h_star_queue.push(
            std::make_tuple(h_star_values[root_node], h_values[root_node], root_node));
    }

    while (!open.empty()) {
        auto top = open.top();
        open.pop();
        int h = top.first;
        int node = top.second;
        int h_star = h_star_values[node];
        generated[node].second = true;

        auto h_star_top = h_star_queue.top();
        int regret_h_star = std::get<0>(h_star_top);
        int regret_h = std::get<1>(h_star_top);
        int regret_node = std::get<2>(h_star_top);
            
        node_log.push_back(node);
        h_log.push_back(h);
        h_star_log.push_back(h_star);
        regret_node_log.push_back(regret_node);
        regret_h_log.push_back(regret_h);
        regret_h_star_log.push_back(regret_h_star);

        if (is_goal[node]) {
            int current = node;
            while (current != -1) {
                path.insert(current);
                current = generated[current].first;
            }
            break;
        }

        // if argmin(h*) is expanded, find the next node which is not expanded yet
        if (regret_node == node) {
            h_star_queue.pop();
            while (!h_star_queue.empty()) {
                auto current = std::get<2>(h_star_queue.top());
                if (generated.find(current) == generated.end() || !generated[current].second)
                    break;
                h_star_queue.pop();
            }
        }

        for (auto successor : adjacent_list[node]) {
            int successor_node = successor.first;
            // do not insert a successor into open if already generated
            if (generated.find(successor_node) == generated.end()) {
                int successor_h = h_values[successor_node];
                if (successor_h == -1) continue;
                generated[successor_node] = std::make_pair(node, false);
                open.push(std::make_pair(successor_h, successor_node));
                int successor_h_star = h_star_values[successor_node];
                if (successor_h_star != -1)
                    h_star_queue.push(
                        std::make_tuple(successor_h_star, successor_h, successor_node));
            }
        }
    }

    std::ofstream output_file(output_filename);
    output_file << "node\th\th*\tpath\tregret node\tregret h\tregret h*" << std::endl; 
    for (int i = 0, n = h_log.size(); i < n; ++i) {
        output_file << node_log[i] << "\t" << h_log[i] << "\t" << h_star_log[i] << "\t";
        if (path.find(node_log[i]) == path.end())
            output_file << "0\t";
        else
            output_file << "1\t";
        output_file << regret_node_log[i] << "\t" << regret_h_log[i] << "\t" << regret_h_star_log[i] << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "2 argumetns are required: [input] [output] [gbfs report suffix (optional)]" << std::endl;
        exit(1);
    }
    std::string input_filename = argv[1];
    std::string output_filename = argv[2];

    std::vector<std::string> h_names;
    std::vector<std::vector<std::pair<int, int>>> adjacent_list;
    std::vector<std::vector<std::pair<int, int>>> inverse_adjacent_list;
    std::vector<std::vector<int>> h_values;
    std::vector<bool> is_goal;
    int root_node = -1;
    try {
        root_node = read_input(input_filename, h_names, adjacent_list, inverse_adjacent_list, h_values, is_goal);
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
    std::vector<int> h_star_values;
    calculate_h_star(inverse_adjacent_list, is_goal, h_star_values);
    write_output(output_filename, h_names, adjacent_list, is_goal, h_values, h_star_values);

    if (argc > 3) {
        std::string gbfs_report_suffix = argv[3];
        for (int i = 0, n = h_names.size(); i < n; ++i) {
            std::string gbfs_report_name = h_names[i] + gbfs_report_suffix;
            report_gbfs(root_node, adjacent_list, is_goal, h_values[i], h_star_values, gbfs_report_name);
        }
    }

}