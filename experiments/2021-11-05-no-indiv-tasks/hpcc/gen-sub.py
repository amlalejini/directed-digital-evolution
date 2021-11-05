'''
Generate slurm job submission scripts - one per condition
'''

import argparse, os, sys, errno, subprocess, csv
from pyvarco import CombinationCollector

seed_offset = 50000
default_num_replicates = 30
job_time_request = "12:00:00"
job_memory_request = "4G"
job_account = "devolab"
job_name = "11-05"
cpus_per_node="32"
executable = "directed-digital-evolution"
base_script_filename = './base_script.txt'

# Create combo object to collect all conditions we'll run
combos = CombinationCollector()
combos.register_var("NUM_POPS")
combos.register_var("EPOCHS")
combos.register_var("UPDATES_PER_EPOCH")
combos.register_var("LOCAL_GRID_WIDTH")
combos.register_var("LOCAL_GRID_HEIGHT")
combos.register_var("SELECTION_METHOD")
combos.register_var("POPULATION_SAMPLING_SIZE")
combos.register_var("AVIDAGP_ENV_FILE")
combos.register_var("TOURNAMENT_SEL_TOURN_SIZE")

combos.add_val(
    "AVIDAGP_ENV_FILE",
    [
        "environment-no-indiv.json",
        "environment-big.json"
    ]
)

combos.add_val(
    "NUM_POPS",
    ["96"]
)

combos.add_val(
    "EPOCHS",
    ["3000"]
)

combos.add_val(
    "UPDATES_PER_EPOCH",
    ["300"]
)

combos.add_val(
    "LOCAL_GRID_WIDTH",
    ["25"]
)

combos.add_val(
    "LOCAL_GRID_HEIGHT",
    ["40"]
)

combos.add_val(
    "SELECTION_METHOD",
    [
        "elite",
        "tournament",
        "lexicase",
        "non-dominated-elite",
        "none"
    ]
)

combos.add_val(
    "POPULATION_SAMPLING_SIZE",
    ["10"]
)

combos.add_val(
    "TOURNAMENT_SEL_TOURN_SIZE",
    ["4"]
)

# Load in the base slurm file
with open(base_script_filename, 'r') as fp:
    base_sub_script = fp.read()

'''
This is functionally equivalent to the mkdir -p [fname] bash command
'''
def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

def main():
    parser = argparse.ArgumentParser(description="Run submission script.")
    parser.add_argument("--data_dir", type=str, help="Where is the output directory for phase one of each run?")
    parser.add_argument("--config_dir", type=str, help="Where is the configuration directory for experiment?")
    parser.add_argument("--replicates", type=int, default=default_num_replicates, help="How many replicates should we run of each condition?")
    parser.add_argument("--job_dir", type=str, help="Where to output these job files?")

    # Load in command line arguments
    args = parser.parse_args()
    data_dir = args.data_dir
    config_dir = args.config_dir
    num_replicates = args.replicates
    job_dir = args.job_dir

    # Get list of all combinations to run
    combo_list = combos.get_combos()
    # Calculate how many jobs we have, and what the last id will be
    num_jobs = num_replicates * len(combo_list)
    print(f'Generating {num_jobs} jobs across {len(combo_list)} files!')

    print("Conditions:")
    for combo in combo_list:
        print(" ", combo)

    # Create job file for each condition
    cur_job_id = 0
    cond_i = 0
    for condition_dict in combo_list:
        cur_seed = seed_offset + (cur_job_id * num_replicates)
        filename_prefix = f'RUN_C{cond_i}_N{condition_dict["NUM_POPS"]}'
        job_name = f"C{cond_i}"
        file_str = base_sub_script
        file_str = file_str.replace("<<JOB_NAME>>", job_name)
        file_str = file_str.replace("<<MEMORY_REQUEST>>", job_memory_request)
        file_str = file_str.replace("<<TIME_REQUEST>>", job_time_request)
        file_str = file_str.replace("<<ACCOUNT_NAME>>", job_account)
        file_str = file_str.replace("<<ARRAY_RANGE>>", f"1-{num_replicates}")
        file_str = file_str.replace("<<CPUS_PER_NODE>>", cpus_per_node)
        file_str = file_str.replace("<<EXEC>>", executable)
        file_str = file_str.replace("<<JOB_SEED_OFFSET>>", str(cur_seed))
        file_str = file_str.replace("<<CONFIG_DIR>>", config_dir)

        # ===================================================
        # ============= CONFIGURE RUN COMMANDS ==============
        # ===================================================
        run_dir = os.path.join(data_dir, f"{filename_prefix}_"+"${SEED}")
        file_str = file_str.replace("<<RUN_DIR>>", run_dir)

        # Format commandline arguments for the run
        # first, just copy over condition dictionary values
        run_param_info = {key:condition_dict[key] for key in condition_dict}
        run_param_info["OUTPUT_SUMMARY_UPDATE_RESOLUTION"] = str(condition_dict["UPDATES_PER_EPOCH"])
        run_param_info["OUTPUT_PHYLOGENY_SNAPSHOT_EPOCH_RESOLUTION"] = "500"
        run_param_info["SEED"] = "${SEED}"
        if (condition_dict["POPULATION_SAMPLING_SIZE"] == "900"):
            run_param_info["POPULATION_SAMPLING_METHOD"] = "full"
        else:
            run_param_info["POPULATION_SAMPLING_METHOD"] = "random"

        fields = list(run_param_info.keys())
        fields.sort()
        run_params = " ".join([f"-{field} {run_param_info[field]}" for field in fields])

        # Add run commands to run the experiment
        run_commands = ''
        run_commands += f'RUN_PARAMS="{run_params}"\n'
        run_commands += 'echo "./${EXEC} ${RUN_PARAMS}" > cmd.log\n'
        run_commands += './${EXEC} ${RUN_PARAMS} > run.log\n'

        file_str = file_str.replace("<<RUN_COMMANDS>>", run_commands)

        # ===================================================
        # Output slurm script
        mkdir_p(job_dir)
        with open(os.path.join(job_dir, f'{filename_prefix}.sb'), 'w') as fp:
            fp.write(file_str)
        cur_job_id += 1
        cond_i += 1

if __name__ == "__main__":
    main()
