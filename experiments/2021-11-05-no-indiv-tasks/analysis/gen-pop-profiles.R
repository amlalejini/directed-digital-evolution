# -- Generate population task profiles --

library(tidyverse)
library(ggplot2)
library(GGally)
library(ggpcp)
library(cowplot)
library(RColorBrewer)
library(scales)
library(viridis)

data_dir <- "/home/amlalejini/devo_ws/directed-digital-evolution/experiments/2021-11-05-no-indiv-tasks/analysis/data/"
dump_dir <- "/home/amlalejini/devo_ws/directed-digital-evolution/experiments/2021-11-05-no-indiv-tasks/analysis/plots/"


# Configure our default graphing theme
theme_set(theme_cowplot())
# Create a directory to store plots
profiles_directory <- paste0(dump_dir, "pop-profiles")
dir.create(profiles_directory, showWarnings=FALSE)

######################################################################
# Load and process experiment summary data
######################################################################
exp_summary_data_loc <- paste0(working_directory,"data/experiment_summary.csv")
exp_summary_data <- read.csv(exp_summary_data_loc, na.strings="NONE")

exp_summary_data$SELECTION_METHOD <- factor(
  exp_summary_data$SELECTION_METHOD,
  levels=c(
    "elite",
    "tournament",
    "lexicase",
    "non-dominated-elite",
    "non-dominated-tournament",
    "random",
    "none"
  ),
  labels=c(
    "elite",
    "tourn",
    "lex",
    "nde",
    "ndt",
    "random",
    "none"
  )
)

exp_summary_data$AVIDAGP_ENV_FILE <- factor(
  exp_summary_data$AVIDAGP_ENV_FILE,
  levels=c(
    "environment-no-indiv.json",
    "environment-big.json"
  ),
  labels=c(
    "env-bb-0",
    "env-bb-1"
  )
)


exp_summary_data$NUM_POPS <- factor(
  exp_summary_data$NUM_POPS,
  levels=c(
    "24",
    "48",
    "96"
  )
)

exp_summary_data$UPDATES_PER_EPOCH <- as.factor(
  exp_summary_data$UPDATES_PER_EPOCH
)

exp_summary_data$TOURNAMENT_SEL_TOURN_SIZE <- as.factor(
  exp_summary_data$TOURNAMENT_SEL_TOURN_SIZE
)

exp_summary_data$POPULATION_SAMPLING_SIZE <- as.factor(
  exp_summary_data$POPULATION_SAMPLING_SIZE
)

exp_summary_data$SAMPLE_SIZE <- exp_summary_data$POPULATION_SAMPLING_SIZE
exp_summary_data$ENV <- exp_summary_data$AVIDAGP_ENV_FILE
exp_summary_data$U_PER_E <- exp_summary_data$UPDATES_PER_EPOCH
######################################################################

######################################################################
# Load and process profile data
######################################################################
population_snapshot_loc <- paste0(working_directory,"data/population_profiles.csv")
population_snapshot_data <- read.csv(population_snapshot_loc, na.strings="NONE")

population_snapshot_data$task_name <- factor(
  population_snapshot_data$task_name,
  levels=c(
    "ECHO_0",
    "NAND_0",
    "NOT_0",
    "OR_NOT_0",
    "AND_0",
    "OR_0",
    "AND_NOT_0",
    "NOR_0",
    "XOR_0",
    "EQU_0",
    "ECHO_1",
    "MATH_1AA_1",
    "MATH_1AB_1",
    "MATH_1AC_1",
    "MATH_2AA_1",
    "MATH_2AB_1",
    "MATH_2AC_1",
    "MATH_2AD_1",
    "MATH_2AE_1",
    "MATH_2AF_1",
    "MATH_2AG_1",
    "MATH_2AH_1"
  )
)


######################################################################


gen_heatmap <- function(profile_data, run_info_data) {
  selection_scheme <- run_info_data$SELECTION_METHOD
  environment <- run_info_data$AVIDAGP_ENV_FILE
  seed <- run_info_data$SEED

  ggplot(
    filter(profile_data, pop_level=="1"),
    aes(x=task_name, y=pop_order, fill=task_score)
  ) +
  geom_tile(
    color="white",
    size=0.1,
    width=1,
    height=1
  ) +
  geom_tile(
    data=filter(profile_data, pop_level=="1" & task_coverage=="0"),
    fill="#666666",
    color="white",
    size=0.1,
    # size=0.2,
    width=1,
    height=1
  ) +
  ggtitle(
    paste(selection_scheme, environment, seed, sep=" | ")
  ) +
  scale_fill_viridis(discrete=FALSE) +
  theme(
    axis.text.x=element_text(angle=45, hjust=1)
  )

  ggsave(
    paste0(
      profiles_directory,
      "/profile_",
      seed,"_",
      selection_scheme,
      ".jpg"
    ),
    width=15,
    height=10
  )
}

######################################################################
# Generate profiles
######################################################################
seeds <- exp_summary_data$SEED
for (seed in seeds) {
  print(paste0("Generating profile for ", seed))
  run_info <- filter(exp_summary_data, SEED==seed)
  if (nrow(run_info)==0) {
    print("  Failed to find experiment summary information.")
    next
  }

  profile_data <- filter(population_snapshot_data, SEED==seed)
  gen_heatmap(profile_data, run_info)
}
