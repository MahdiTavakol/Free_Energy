#!/bin/bash

# List all folders
folders=(run-*)

# Total number of folders
total_folders=${#folders[@]}

# Number of .sh files to create
num_files=25

# Number of folders per .sh file
folders_per_file=2

# Loop to create .sh files
for ((i=0; i<num_files; i++)); do
  # Start writing to a new .sh file
  sh_file="script_$((i + 1)).sh"
  echo "#!/bin/bash" > "$sh_file"
  echo "# Running LAMMPS on ARC" >> "$sh_file"
  echo "#SBATCH -J SMD_$((i + 1))" >> "$sh_file"
  echo "#SBATCH --nodes=1" >> "$sh_file"
  echo "#SBATCH --ntasks-per-node=48" >> "$sh_file"
  echo "#SBATCH --mem-per-cpu=2G" >> "$sh_file"
  echo "#SBATCH --partition=long" >> "$sh_file"
  echo "#SBATCH --time=4-00:00:00" >> "$sh_file"
  echo "#SBATCH -o SMD_$((i + 1)).out" >> "$sh_file"
  echo "#SBATCH -e SMD_$((i + 1)).err" >> "$sh_file"
  echo "#SBATCH --mail-type=BEGIN,END" >> "$sh_file"
  echo "#SBATCH --mail-user=mahditavakol90@gmail.com" >> "$sh_file"
  echo "" >> "$sh_file"
  echo "module purge *" >> "$sh_file"
  echo "module load imkl-FFTW/2023.1.0-iimpi-2023.03" >> "$sh_file"
  echo "module load iimpi/2023.03" >> "$sh_file"
  echo "" >> "$sh_file"

  # Add folder-specific commands
  start=$((i * folders_per_file))
  end=$((start + folders_per_file))
  for ((j=start; j<end && j<total_folders; j++)); do
    folder_name=${folders[j]}
    run_number=${folder_name#run-}  # Extract the number part of the folder name
    echo "cd ${folder_name}" >> "$sh_file"
    echo "chmod +x ./lmp-SMD-ARC" >> "$sh_file"
    echo "echo 'Running task in ${folder_name}'" >> "$sh_file"
    echo "mpirun -np 48 ./lmp-SMD-ARC -in SMD.in -sf intel -pk intel 0 omp 1 mode single -var frame ${run_number}" >> "$sh_file"
    echo "" >> "$sh_file"
    echo "" >> "$sh_file"
    
    echo "" >> "$sh_file"
    echo "" >> "$sh_file"
    echo "echo 'Running task in ${folder_name}'" >> "$sh_file"
    echo "mpirun -np 48 ./lmp-SMD-ARC -in SMD-reverse.in -sf intel -pk intel 0 omp 1 mode single -var frame ${run_number}" >> "$sh_file"
    echo "cd .." >> "$sh_file"
    echo "" >> "$sh_file"
  done
done
