'''
Aggregate experiment data
- Experiment: AvidaGP L9

SUMMARY FILES
- experiment
    - config + summary(evaluation) + systematics + summary(world summary)
- world
- task

'''

import argparse, os, sys, errno, csv, json, copy
from scipy.stats import entropy

run_identifiers = ["RUN_"] # String that identifies a run directory
default_trait_cov_thresh = 100

systematics_fields = {
    "num_taxa",
    "total_orgs",
    "ave_depth",
    "num_roots",
    "mrca_depth",
    "diversity",
    "mean_genotype_pairwise_distance",
    "min_genotype_pairwise_distance",
    "max_genotype_pairwise_distance",
    "variance_genotype_pairwise_distance",
    "genotype_current_phylogenetic_diversity"
}

world_eval_fields = {
    "epoch",
    "aggregate_scores",
    "scores",
    "selected",
    "num_unique_selected"
}

time_series_config_fields = {
    "SELECTION_METHOD",
    "NUM_POPS",
    "UPDATES_PER_EPOCH",
    "POPULATION_SAMPLING_SIZE",
    "SEED"
}
time_series_world_eval_fields = {
    "epoch",
    "num_unique_selected",
    "updates_elapsed"
}

def read_csv(file_path):
    content = None
    with open(file_path, "r") as fp:
        content = fp.read().strip().split("\n")
    header = content[0].split(",")
    content = content[1:]
    lines = [{header[i]: l[i] for i in range(len(header))} for l in csv.reader(content, quotechar='"', delimiter=',', quoting=csv.QUOTE_ALL, skipinitialspace=True)]
    return lines

# NOTE - this function assumes that data will be ordered!
def filter_ordered_data(data, units, resolution):
    if (units == "interval"):
        return filter_ordered_data_interval(data, resolution)
    elif (units == "total"):
        return filter_ordered_data_total(data, resolution)
    elif (units == "epoch"):
        return filter_ordered_data_keyval(data, "epoch", resolution)
    elif (units == "update"):
        return filter_ordered_data_keyval(data, "updates_elapsed", resolution)
    else:
        return data

def filter_ordered_data_keyval(data, key, interval):
    prev_epoch = 0
    ret_data = []
    for i in range(len(data)):
        cur_epoch = int(data[i][key])
        if (i==0) or cur_epoch >= (prev_epoch+interval) or (i==(len(data)-1)):
            ret_data.append(copy.deepcopy(data[i]))
            prev_epoch = cur_epoch
    return ret_data

def filter_ordered_data_interval(data, interval):
    return [data[i] for i in range(len(data)) if (i==0) or (not (i % interval)) or (i==len(data)-1)]

def filter_ordered_data_total(data, total):
    sample = [int(x*(len(data)-1)/(total-1)) for x in range(total)]
    return [data[i] for i in sample]

def main():
    parser = argparse.ArgumentParser(description="Run submission script.")
    parser.add_argument("--data_dir", type=str, help="Where is the base output directory for each run?")
    parser.add_argument("--dump", type=str, help="Where to dump this?", default=".")
    parser.add_argument("--trait_cov_thresh", type=float, help="Threshold for trait score to count toward trait coverage", default=default_trait_cov_thresh)
    parser.add_argument("--units", type=str, default="epoch", choices=["epoch", "update", "interval", "total"], help="Unit for resolution of time series")
    parser.add_argument("--resolution", type=int, default=1, help="What resolution should we collect time series data at?")

    # Parse user arguments
    args = parser.parse_args()
    data_dir = args.data_dir
    dump_dir = args.dump
    time_series_units = args.units
    time_series_resolution = args.resolution
    trait_cov_thresh = args.trait_cov_thresh

    # Does data directory exist, if not exit.
    if not os.path.exists(data_dir):
        print("Unable to find data directory.")
        exit(-1)

    if time_series_resolution < 1:
        print("Time series resolution must be >= 1")
        exit(-1)

    # Create directory to dump output
    os.makedirs(name=dump_dir, exist_ok=True)

    # Aggregate run directories
    run_dirs = [run_dir for run_dir in os.listdir(data_dir) if any([rid in run_dir for rid in run_identifiers])]
    print(f"Found {len(run_dirs)} run directories.")

    # TODO - create time series file(s)
    evaluation_time_series_header = None
    evaluation_time_series_content_lines = []
    world_summary_time_series_header = None
    world_summary_time_series_content_lines = []

    # The summary file contains one line per run
    experiment_summary_header = None
    experiment_summary_content_lines = []

    evaluation_time_series_fpath = os.path.join(dump_dir, "evaluation_time_series.csv")
    with open(evaluation_time_series_fpath, "w") as fp:
        fp.write("")

    # world_time_series_fpath = os.path.join(dump_dir, "world_summary_time_series.csv")
    # with open(world_time_series_fpath)

    # For each run directory,
    # - get configuration, TODO
    skipped_runs = []
    for run_dir in run_dirs:
        run_path = os.path.join(data_dir, run_dir)

        experiment_summary_info = {} # Hold summary information about this run.
        eval_time_series_info = []
        print(f"Processing: {run_path}")
        # Add generic info
        experiment_summary_info["trait_cov_thresh"] = trait_cov_thresh

        #######################################################################
        # Extract run configuration (from output/run_config.csv)
        #######################################################################
        run_config_path = os.path.join(run_path, "output", "run_config.csv")
        run_config_data = read_csv(run_config_path)
        # Add each experiment-wide parameter to summary info
        for line in run_config_data:
            if line["source"] == "experiment":
                experiment_summary_info[line["parameter"]] = line["value"]
        # Accumulate world-level parameter names
        world_params = {line["parameter"]:set() for line in run_config_data if "world_" in line["source"]}
        # Collect the distinct values for each world-level parameter
        for line in run_config_data:
            param = line["parameter"]
            if param in world_params:
                world_params[param].add(line["value"])
        # Only add world-level parameters to summary info if value is shared across all worlds
        for param in world_params:
            if len(world_params[param]) == 1:
                experiment_summary_info[param] = list(world_params[param])[0]
        # Grab a few useful parameter values
        num_pops = int(experiment_summary_info["NUM_POPS"])
        max_pop_size = int(experiment_summary_info["world_size"])
        max_update = int(experiment_summary_info["UPDATES_PER_EPOCH"])
        track_systematics = bool(int(experiment_summary_info["TRACK_SYSTEMATICS"]))
        epoch = int(experiment_summary_info["EPOCHS"])
        total_updates = epoch*max_update

        # Add some secondarily-computed configuration details
        experiment_summary_info["total_updates"] = str(total_updates)

        #######################################################################
        # Extract run systematics
        #######################################################################
        if track_systematics:
            run_sys_path = os.path.join(run_path, "output", "systematics.csv")
            run_sys_data = read_csv(run_sys_path)
            # Extract target epoch systematics data
            targ_epoch_run_sys_data = [line for line in run_sys_data if line["epoch"]==str(epoch)]
            if len(targ_epoch_run_sys_data) != 1:
                print("  Failed to find target epoch, or too many target lines.", targ_epoch_run_sys_data)
                skipped_runs.append(run_dir)
                continue
            targ_epoch_run_sys_data = targ_epoch_run_sys_data[0]
            # Add systematics fields to experiment summary
            for field in systematics_fields:
                experiment_summary_info[field] = targ_epoch_run_sys_data[field]
            # TODO - time series data!

            # Clear out data list
            run_sys_data = None

        #######################################################################
        # Extract run world evaluation data
        #######################################################################
        run_world_eval_path = os.path.join(run_path, "output", "world_evaluation.csv")
        run_world_eval_data = read_csv(run_world_eval_path)
        # Extract target epoch evaluation data
        targ_epoch_run_world_eval_data = [line for line in run_world_eval_data if line["epoch"]==str(epoch)]
        if len(targ_epoch_run_world_eval_data) != 1:
            print("  Failed to find target epoch, or too many target lines.", targ_epoch_run_world_eval_data)
            skipped_runs.append(run_dir)
            continue
        targ_epoch_run_world_eval_data = targ_epoch_run_world_eval_data[0]
        # Add world evaluation fields to experiment summary
        for field in world_eval_fields:
            experiment_summary_info[field] = targ_epoch_run_world_eval_data[field]
        # Compute secondary experiment summary fields
        aggregate_scores = json.loads(targ_epoch_run_world_eval_data["aggregate_scores"])
        scores = json.loads(targ_epoch_run_world_eval_data["scores"])
        # Based on target epoch
        # max_aggregate_score
        max_aggregate_score = max(aggregate_scores)
        experiment_summary_info["max_aggregate_score"] = max_aggregate_score
        # total_trait_coverage
        trait_coverage = {j for world_scores in scores for j in range(len(world_scores)) if world_scores[j] >= trait_cov_thresh}
        total_trait_coverage = len(trait_coverage)
        experiment_summary_info["total_trait_coverage"] = total_trait_coverage
        # max_trait_coverage
        by_world_trait_coverage = [len([j for j in range(len(world_scores)) if world_scores[j] >= trait_cov_thresh]) for world_scores in scores]
        max_trait_coverage = max(by_world_trait_coverage)
        experiment_summary_info["max_trait_coverage"] = max_trait_coverage
        # Based on whole run (<= epoch)
        unique_selected = []
        entropy_selected = []
        for line in run_world_eval_data:
            if int(line["epoch"]) > epoch: continue
            num_unique_selected = int(line["num_unique_selected"])
            unique_selected.append(num_unique_selected)
            selected = json.loads(line["selected"])
            selected_dist = [0 for _ in range(num_pops)]
            for id in selected:
                selected_dist[int(id)] += 1
            entropy_selected.append(entropy(selected_dist, base=2))
        avg_unique_selected = sum(unique_selected) / len(unique_selected)
        avg_entropy_selected = sum(entropy_selected) / len(entropy_selected)
        experiment_summary_info["avg_unique_selected"] = avg_unique_selected
        experiment_summary_info["avg_entropy_selected"] = avg_entropy_selected # NOTE - not sure if this is a good way of getting at this

        # TIME SERIES AGGREGATE
        # Add updates elapsed to each data entry
        for line in run_world_eval_data:
            line["updates_elapsed"] = (int(line["epoch"])*max_update)+max_update
        # Filter data entries baseds on requested units/interval
        time_series_eval_data = filter_ordered_data(run_world_eval_data, time_series_units, time_series_resolution)
        eval_filtered_epochs = []
        # Add line to file for each entry
        for i in range(len(time_series_eval_data)):
            info = {}
            data_i = time_series_eval_data[i]
            eval_filtered_epochs.append(data_i["epoch"])
            # Config fields are shared for all lines
            for field in time_series_config_fields: info[field] = experiment_summary_info[field]
            # Eval fields carry over from current line
            for field in time_series_world_eval_fields: info[field] = data_i[field]
            # Calculated fields
            info["entropy_selected"] = entropy_selected[i]
            aggregate_scores = json.loads(data_i["aggregate_scores"])
            scores = json.loads(data_i["scores"])
            max_aggregate_score = max(aggregate_scores)
            trait_coverage = {j for world_scores in scores for j in range(len(world_scores)) if world_scores[j] >= trait_cov_thresh}
            total_trait_coverage = len(trait_coverage)
            by_world_trait_coverage = [len([j for j in range(len(world_scores)) if world_scores[j] >= trait_cov_thresh]) for world_scores in scores]
            max_trait_coverage = max(by_world_trait_coverage)

            info["max_aggregate_score"] = max_aggregate_score
            info["total_trait_coverage"] = total_trait_coverage
            info["max_trait_coverage"] = max_trait_coverage

            # fields = info.keys()
            # fields.sort()
            # evaluation_time_series_content_lines.append(",".join([str(info[field]) for field in fields]))
            eval_time_series_info.append(info)

        # Clear out data list
        run_world_eval_data = None

        #######################################################################
        # Extract run world summary data
        #######################################################################
        run_world_summary_path = os.path.join(run_path, "output", "world_summary.csv")
        run_world_summary_data = read_csv(run_world_summary_path)

        # Extract target epoch summary data
        # NOTE - if we're not tracking systematics, max update is one off because we don't update the file before the world's update function is called the final time
        run_world_summary_data_by_epoch = {}
        for line in run_world_summary_data:
            if line["epoch"] in run_world_summary_data_by_epoch:
                run_world_summary_data_by_epoch[line["epoch"]].append(line)
            else:
                run_world_summary_data_by_epoch[line["epoch"]] = [line]

        targ_epoch_run_world_summary_data = [line for line in run_world_summary_data if line["epoch"]==str(epoch) and line["world_update"]==str(max_update+int(not track_systematics))]

        if len(targ_epoch_run_world_summary_data) != num_pops:
            print("  Failed to find target epoch, or too many target lines.", targ_epoch_run_world_summary_data)
            skipped_runs.append(run_dir)
            continue
        # Experiment summary info
        # average num orgs
        num_orgs = [int(line["num_orgs"]) for line in targ_epoch_run_world_summary_data]
        avg_num_orgs = sum(num_orgs) / len(num_orgs)
        experiment_summary_info["avg_num_orgs"] = avg_num_orgs
        # average gen
        gens = [float(line["avg_generation"]) for line in targ_epoch_run_world_summary_data]
        avg_gens = sum(gens) / len(gens)
        experiment_summary_info["avg_gens"] = avg_gens
        # average cpu cycles / replication
        cycles = [float(line["avg_cpu_cycles_per_replication"]) for line in targ_epoch_run_world_summary_data]
        avg_cycles = sum(cycles) / len(cycles)
        experiment_summary_info["avg_cpu_cycles_per_replication"] = avg_cycles

        # AGGREGATE TIME SERIES (EVALUATION SERIES FILE)
        for i in range(len(eval_filtered_epochs)):
            cur_epoch = str(eval_filtered_epochs[i])
            summary_data = [line for line in run_world_summary_data_by_epoch[cur_epoch] if line["world_update"]==str(max_update+int(not track_systematics))]
            # average num orgs
            num_orgs = [int(line["num_orgs"]) for line in summary_data]
            avg_num_orgs = sum(num_orgs) / len(num_orgs)
            eval_time_series_info[i]["avg_num_orgs"] = avg_num_orgs
            # average gen
            gens = [float(line["avg_generation"]) for line in summary_data]
            avg_gens = sum(gens) / len(gens)
            eval_time_series_info[i]["avg_gens"] = avg_gens
            # average cpu cycles / replication
            cycles = [float(line["avg_cpu_cycles_per_replication"]) for line in summary_data]
            avg_cycles = sum(cycles) / len(cycles)
            eval_time_series_info[i]["avg_cpu_cycles_per_replication"] = avg_cycles

        # Clear out data list
        run_world_summary_data = None

        #######################################################################
        # Dump time series content
        #######################################################################
        time_series_fields = eval_time_series_info[0].keys()
        for i in range(len(eval_time_series_info)):
            evaluation_time_series_content_lines.append(",".join([str(eval_time_series_info[i][field]) for field in time_series_fields]))
        write_header = evaluation_time_series_header == None
        if write_header:
            evaluation_time_series_header = ",".join(time_series_fields)
        elif evaluation_time_series_header != ",".join(time_series_fields):
            print("  Time series header mismatch!")
            exit(-1)
        with open(evaluation_time_series_fpath, "a") as fp:
            if write_header: fp.write(evaluation_time_series_header)
            fp.write("\n")
            fp.write("\n".join(evaluation_time_series_content_lines))
        evaluation_time_series_content_lines = []

        #######################################################################
        # Add experiment summary info to experiment summary file content
        #######################################################################
        experiment_summary_fields = list(experiment_summary_info.keys())
        experiment_summary_fields.sort()
        if experiment_summary_header == None:
            experiment_summary_header = experiment_summary_fields
        elif experiment_summary_header != experiment_summary_fields:
            print("Header mismatch!")
            exit(-1)
        # If field contains commas, surround in quotation marks
        for field in experiment_summary_fields:
            if "," in str(experiment_summary_info[field]):
                experiment_summary_info[field] = f'"{str(experiment_summary_info[field])}"'
        experiment_summary_line = [str(experiment_summary_info[field]) for field in experiment_summary_fields]
        experiment_summary_content_lines.append(",".join(experiment_summary_line))

    # --> END RUNS FOR LOOP <--
    # write out time series data

    # write out aggregate data
    with open(os.path.join(dump_dir, "experiment_summary.csv"), "w") as fp:
        out_content = ",".join(experiment_summary_header) + "\n" + "\n".join(experiment_summary_content_lines)
        fp.write(out_content)

    print(f"Skipped ({len(skipped_runs)}):")
    for run_dir in skipped_runs:
        print(f" - {run_dir}")

if __name__ == "__main__":
    main()