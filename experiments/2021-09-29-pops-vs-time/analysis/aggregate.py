'''
Aggregate experiment data
- Experiment: AvidaGP L9

SUMMARY FILES
- experiment
    - config + summary(evaluation) + systematics + summary(world summary)
- world
- task

'''

import argparse, os, sys, errno, csv, json
from scipy.stats import entropy

run_identifier = "RUN_" # String that identifies a run directory
default_trait_cov_thresh = 100

trait_order = ["echo","nand","not","or_not","and","or","and_not","nor","xor","equ"]


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


def read_csv(file_path):
    content = None
    with open(file_path, "r") as fp:
        content = fp.read().strip().split("\n")
    header = content[0].split(",")
    content = content[1:]
    lines = [{header[i]: l[i] for i in range(len(header))} for l in csv.reader(content, quotechar='"', delimiter=',', quoting=csv.QUOTE_ALL, skipinitialspace=True)]
    return lines

def main():
    parser = argparse.ArgumentParser(description="Run submission script.")
    parser.add_argument("--data_dir", type=str, help="Where is the base output directory for each run?")
    parser.add_argument("--dump", type=str, help="Where to dump this?", default=".")
    # parser.add_argument("--epoch", type=int, help="Epoch to pull data for?")
    parser.add_argument("--epoch_range", type=int, help="The range (in epochs) to collect time series data?", nargs=2)
    parser.add_argument("--trait_cov_thresh", type=float, help="Threshold for trait score to count toward trait coverage", default=default_trait_cov_thresh)

    # Parse user arguments
    args = parser.parse_args()
    data_dir = args.data_dir
    dump_dir = args.dump
    # epoch = args.epoch
    epoch_range = args.epoch_range
    trait_cov_thresh = args.trait_cov_thresh

    # Does data directory exist, if not exit.
    if not os.path.exists(data_dir):
        print("Unable to find data directory.")
        exit(-1)

    # Create directory to dump output
    os.makedirs(name=dump_dir, exist_ok=True)

    # Aggregate run directories
    run_dirs = [run_dir for run_dir in os.listdir(data_dir) if run_identifier in run_dir]
    print(f"Found {len(run_dirs)} run directories.")

    # TODO - create time series file(s)

    # The summary file contains one line per run
    experiment_summary_header = None
    experiment_summary_content_lines = []

    # For each run directory,
    # - get configuration, TODO
    skipped_runs = []
    for run_dir in run_dirs:
        run_path = os.path.join(data_dir, run_dir)

        experiment_summary_info = {} # Hold summary information about this run.
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

        # Clear out data list
        run_world_eval_data = None
        #######################################################################
        # Extract run world summary data
        #######################################################################
        run_world_summary_path = os.path.join(run_path, "output", "world_summary.csv")
        run_world_summary_data = read_csv(run_world_summary_path)

        # Extract target epoch summary data
        # NOTE - if we're not tracking systematics, max update is one off because we don't update the file before the world's update function is called the final time
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

        # Clear out data list
        run_world_summary_data = None

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
    # write out aggregate data
    with open(os.path.join(dump_dir, "experiment_summary.csv"), "w") as fp:
        out_content = ",".join(experiment_summary_header) + "\n" + "\n".join(experiment_summary_content_lines)
        fp.write(out_content)

    print(f"Skipped ({len(skipped_runs)}):")
    for run_dir in skipped_runs:
        print(f" - {run_dir}")

if __name__ == "__main__":
    main()