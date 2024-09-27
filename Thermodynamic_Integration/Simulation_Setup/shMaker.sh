#!/bin/bash

# Base job name
base_jobname="178TI"
# Array of increments
increments=(0-00 0-02 0-04 0-06 0-08 0-10 0-12 0-14 0-16 0-18 0-20 0-22 0-24 0-26 0-28 0-30 0-32 0-34 0-36 0-38 0-40 0-42 0-44 0-46 0-48 0-50 0-52 0-54 0-56 0-58 0-60 0-62 0-64 0-66 0-68 0-70 0-72 0-74 0-76 0-78 0-80 0-82 0-84 0-86 0-88 0-90 0-92 0-94 0-96 0-98 1-00)

# Function to create a single job file
create_job_file() {
    local jobname=$1
    local params=("${@:2}")

    cat <<EOL > "${jobname}.slurm"
#!/bin/bash
#SBATCH --job-name=${jobname}
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=128
#SBATCH --cpus-per-task=1
#SBATCH --time=3-00:00:00
#SBATCH -e ${jobname}.err
#SBATCH -o ${jobname}.out
#SBATCH  --mail-type=BEGIN,END
#SBATCH  --mail-user=mahditavakol90@gmail.com
#SBATCH --account=e05-biosoft-tan
#SBATCH --partition=standard
#SBATCH --qos=long
#SBATCH --exclusive

module load PrgEnv-aocc

# Base directory where the folders are located
base_dir="/mnt/lustre/a2fs-nvme/work/e280/e280/mthecbio/1-Simulations/178/2-TI/"

# Function to run a single simulation
run_simulation() {
    local sim_dir=\$1
    local sim_executable="./lmp-archer2-cray-clang-metaAR-v7.5.03"
    local input_file="TI.in"

    cd \$sim_dir
    srun --exclusive --ntasks=32 --cpus-per-task=1 \$sim_executable -in \$input_file -var RAND1 \$RANDOM &
    sleep 20
    cd -
}

# Run simulations for the following increments: ${params[@]}
params=(${params[@]})
for increment in "\${params[@]}"
do
    sim_dir="\$base_dir/\$increment"
    run_simulation \$sim_dir
done

# Wait for all background jobs to finish
wait

echo "Job ${jobname} with increments ${params[@]} has finished running."
EOL
}

# Number of increments per job file
increments_per_job=8

# Generate job files
counter=1
for ((i=0; i<${#increments[@]}; i+=increments_per_job)); do
    jobname="${base_jobname}${counter}"
    params=("${increments[@]:i:increments_per_job}")
    create_job_file "${jobname}" "${params[@]}"
    ((counter++))
done
