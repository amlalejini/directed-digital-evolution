import argparse, os, sys, errno, csv, json, copy, pandas
from scipy.stats import entropy
from scipy.spatial import distance

run_identifiers = ["RUN_"] # String that identifies a run directory

run_config_drop_fields = [
    "indiv_tasks"
]

pop_snapshot_time_series_include_fields = [
    "SEED"
]

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

    args = parser.parse_args()
    data_dir = args.data_dir
    dump_dir = args.dump

    # Does data directory exist, if not exit.
    if not os.path.exists(data_dir):
        print("Unable to find data directory.")
        exit(-1)

    # Create directory to dump output
    os.makedirs(name=dump_dir, exist_ok=True)

    # Aggregate run directories
    run_dirs = [run_dir for run_dir in os.listdir(data_dir) if any([rid in run_dir for rid in run_identifiers])]
    print(f"Found {len(run_dirs)} run directories.")

    # The summary file contains one line per run
    experiment_summary_header = None
    experiment_summary_content_lines = []

    pop_snapshot_time_series_header = None
    pop_snapshot_time_series_content_lines = []

    pop_snapshot_time_series_fpath = os.path.join(dump_dir, "pop_snapshot_time_series.csv")
    with open(pop_snapshot_time_series_fpath, "w") as fp:
        fp.write("")

    # For each run directory,
    total_runs = len(run_dirs)
    cur_run_i = 0
    skipped_runs = []
    for run_dir in run_dirs:
        run_path = os.path.join(data_dir, run_dir)
        experiment_summary_info = {} # Hold summary information about this run.
        pop_snapshot_info = []

        cur_run_i += 1
        print(f"Processing ({cur_run_i}/{total_runs}): {run_path}")

        run_config_path = os.path.join(run_path, "output", "run_config.csv")
        run_config_data = read_csv(run_config_path)
        # Add each experiment-wide parameter to summary info
        for line in run_config_data:
            if line["parameter"] in run_config_drop_fields: continue
            experiment_summary_info[line["parameter"]] = line["value"]

        # Grab a few useful parameter values
        max_update = int(experiment_summary_info["GENS"])
        num_tasks = int(experiment_summary_info["total_tasks"])
        pop_size = int(experiment_summary_info["POP_SIZE"])

        #######################################################################
        # Extract run summary data
        #######################################################################
        run_summary_path = os.path.join(run_path, "output", "world_summary.csv")
        run_summary_data = read_csv(run_summary_path)

        # Identify either: (1) first update a solution appears or (2) the final update
        solution_lines = [int(line["update"]) for line in run_summary_data if line["max_fit_is_solution"]=="1"]
        solution_found = len(solution_lines) > 0
        target_update = max_update
        if solution_found:
            solution_lines.sort()
            target_update = solution_lines[0]
        target_update_summary_data = [line for line in run_summary_data if int(line["update"]) == target_update]
        if len(target_update_summary_data) != 1:
            print(f"  Failed to find target update {target_update} or too many target lines.", target_update_summary_data)
            skipped_runs.append(run_dir)
            continue
        target_update_summary_data = target_update_summary_data[0]
        # Add world evaluation fields to experiment summary
        for field in target_update_summary_data:
            experiment_summary_info[field] = target_update_summary_data[field]
        # Compute any secondary fields
        task_scores = json.loads(target_update_summary_data["max_fit_scores_by_task"])
        num_tasks_covered = len([i for i in range(len(task_scores)) if task_scores[i] > 0])
        experiment_summary_info["max_fit_num_tasks_covered"] = num_tasks_covered

        run_summary_data = None
        #######################################################################
        # Extract population snapshot data
        #######################################################################
        pop_snapshot_path = os.path.join(run_path, "output", "population_snapshot.csv")
        pop_snapshot_data = read_csv(pop_snapshot_path)

        # organize by update
        pop_snapshot_data_by_update = {}
        for line in pop_snapshot_data:
            update = int(line["update"])
            if not update in pop_snapshot_data_by_update:
                pop_snapshot_data_by_update[update] = []
            pop_snapshot_data_by_update[update].append(line)

        # updates
        pop_snapshot_updates = [u for u in pop_snapshot_data_by_update.keys()]
        pop_snapshot_updates.sort()
        # print(pop_snapshot_updates)

        for update in pop_snapshot_updates:
            info = {}
            info["update"] = update
            for field in pop_snapshot_time_series_include_fields:
                info[field] = experiment_summary_info[field]

            # Collect organism task profiles and compute population coverage, max_organism_coverage, and max_organism_aggregate_score
            snapshot = pop_snapshot_data_by_update[update]
            unique_task_profiles = set()
            max_coverage = None
            max_aggregate_score = None
            population_task_coverage = [False for _ in range(num_tasks)]
            for org in snapshot:
                org["scores_by_task"] = list(map(int, org["scores_by_task"].strip('"[]"').split(",")))
                org["scores_by_task_str"] = "".join(map(str, org["scores_by_task"]))
                # Add to task profile set
                unique_task_profiles.add(org["scores_by_task_str"])
                # OR with population task coverage
                org_tasks_covered = 0
                for i in range(len(org["scores_by_task"])):
                    population_task_coverage[i] = population_task_coverage[i] or (org["scores_by_task"][i] > 0)
                    org_tasks_covered += int(org["scores_by_task"][i] > 0)
                if max_coverage==None:
                    max_coverage = org_tasks_covered
                elif org_tasks_covered > max_coverage:
                    max_coverage = org_tasks_covered
                if max_aggregate_score==None:
                    max_aggregate_score = float(org["aggregate_score"])
                elif float(org["aggregate_score"]) > max_aggregate_score:
                    max_aggregate_score = float(org["aggregate_score"])
            population_coverage = sum(map(int, population_task_coverage))
            info["population_task_coverage"] = population_coverage
            info["max_org_task_coverage"] = max_coverage
            info["max_org_aggregate_score"] = max_aggregate_score
            info["num_unique_task_profiles"] = len(unique_task_profiles)

            # -- cosine distance --
            # Calculate cosine centroid
            vec_lens = [ sum(org["scores_by_task"][task_i] for task_i in range(num_tasks)) for org in snapshot]
            pop_norm_task_performances = [ [ 0 if vec_lens[org_i] == 0 else (snapshot[org_i]["scores_by_task"][task_i] / vec_lens[org_i]) for task_i in range(num_tasks)] for org_i in range(pop_size)]
            centroid_norm_task_vector = [ sum([pop_norm_task_performances[org_i][task_i] for org_i in range(pop_size)])/pop_size for task_i in range(num_tasks) ]
            # Normalize the centroid norm task vector
            vec_len = sum(centroid_norm_task_vector)
            for i in range(0, len(centroid_norm_task_vector)):
                if vec_len != 0:
                    centroid_norm_task_vector[i] /= vec_len

            # Calculate each organism's distance to centroid
            pop_cosine_dist_from_centroid = [None if vec_lens[org_i] == 0 else distance.cosine(pop_norm_task_performances[org_i], centroid_norm_task_vector) for org_i in range(pop_size)]
            avg_cosine_dist_from_centroid = [val for val in pop_cosine_dist_from_centroid if val != None]
            if not len(avg_cosine_dist_from_centroid):
                avg_cosine_dist_from_centroid = "NONE"
            else:
                avg_cosine_dist_from_centroid = sum(avg_cosine_dist_from_centroid) / len(avg_cosine_dist_from_centroid)
            info["avg_cosine_dist_from_centroid"] = avg_cosine_dist_from_centroid

            # Add line to population snapshot info
            pop_snapshot_info.append({key:str(info[key]) for key in info})

        #######################################################################
        # Dump time series content
        #######################################################################
        time_series_fields = list(pop_snapshot_info[0].keys())
        time_series_fields.sort()
        for i in range(len(pop_snapshot_info)):
            pop_snapshot_time_series_content_lines.append(",".join([str(pop_snapshot_info[i][field]) for field in time_series_fields]))
        write_header = pop_snapshot_time_series_header == None
        if write_header:
            pop_snapshot_time_series_header = ",".join(time_series_fields)
        elif pop_snapshot_time_series_header != ",".join(time_series_fields):
            print("  Time series header mismatch!")
            exit(-1)
        with open(pop_snapshot_time_series_fpath, "a") as fp:
            if write_header: fp.write(pop_snapshot_time_series_header)
            fp.write("\n")
            fp.write("\n".join(pop_snapshot_time_series_content_lines))
        pop_snapshot_time_series_content_lines = []

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



if __name__ == "__main__":
    main()