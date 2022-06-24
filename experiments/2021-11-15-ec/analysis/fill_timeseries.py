"""
Because we stop EC runs early, time series data ends early for any runs where a solution is found before the final generation.

This script loads a time series, identifies the maximum update represented, and fills out incomplete time series by copying the "solution" line.
"""

import os, csv, copy


resolution = 1000
max_update = 55000
in_fname="data/pop_snapshot_time_series.csv"
out_fname="data/pop_snapshot_time_series_corrected.csv"

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
        data_by_seed[seed][int(line["update"])] = line

    max_update_by_seed = {seed:max([int(data_by_seed[seed][update]["update"]) for update in data_by_seed[seed]]) for seed in data_by_seed}

    corrected_data = []
    update_sequence = [i for i in range(0, max_update+1, resolution)]
    print(update_sequence)
    print(max_update_by_seed)

    for seed in data_by_seed:
        seed_data = data_by_seed[seed]
        max_seed_update = max_update_by_seed[seed]
        for update in update_sequence:
            if update > max_seed_update:
                # Add line from max_update by seed
                corrected_data.append(copy.deepcopy(seed_data[max_seed_update]))
                corrected_data[-1]["update"] = str(update) # fix the update
            else:
                corrected_data.append(copy.deepcopy(seed_data[update]))

    fields = list(data[0].keys())
    fields.sort()

    content = ",".join(fields) + "\n"
    content += "\n".join( [ ",".join([str(line[field]) for field in fields])  for line in corrected_data] )
    with open(out_fname, "w") as fp:
        fp.write(content)





    print(max_update)
    print(len(seeds))



if __name__ == "__main__":
    main()


