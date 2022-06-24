"""
Time series data starts after first round of artificial selection.

This script backfills each time series to start at 0.
"""

import os, csv, copy


in_fname="data/evaluation_time_series.csv"
out_fname="data/evaluation_time_series_corrected.csv"

min_update = 200

def read_csv(file_path):
    content = None
    with open(file_path, "r") as fp:
        content = fp.read().strip().split("\n")
    header = content[0].split(",")
    content = content[1:]
    lines = [{header[i]: l[i] for i in range(len(header))} for l in csv.reader(content, quotechar='"', delimiter=',', quoting=csv.QUOTE_ALL, skipinitialspace=True)]
    return lines

def main():
    data = read_csv(in_fname)
    seeds = {line["SEED"] for line in data}
    data_by_seed = {}
    for line in data:
        seed = line["SEED"]
        if not seed in data_by_seed:
            data_by_seed[seed] = {}
        data_by_seed[seed][int(line["updates_elapsed"])] = line

    fields = list(data[0].keys())
    fields.sort()

    corrected_data = []

    fields_to_zero_out = [
        "avg_cpu_cycles_per_replication",
        "avg_gens",
        "avg_num_orgs",
        "avg_org_fitness",
        "entropy_selected",
        "generations_elapsed",
        "max_aggregate_score",
        "max_trait_coverage",
        "num_pop_trait_profiles",
        "num_unique_selected",
        "pop_trait_profile_entropy",
        "total_trait_coverage"
    ]
    for seed in data_by_seed:
        data_200 = data_by_seed[seed][200]
        data_0 = copy.deepcopy(data_200)
        data_0["updates_elapsed"] = "0"
        data_0["epoch"] = "-1"
        for field in fields_to_zero_out:
            data_0[field] = "0"
        data.append(data_0)



    content = ",".join(fields) + "\n"
    content += "\n".join( [ ",".join([str(line[field]) for field in fields])  for line in data] )
    with open(out_fname, "w") as fp:
        fp.write(content)

    print(len(seeds))

if __name__ == "__main__":
    main()


