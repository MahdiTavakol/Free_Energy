#!/usr/bin/env python3

import re
import sys

def modify_coeffs(input_file_path, output_file_path, LJ1s, LJ2s, Masses):
    """
    Modify the PairIJ Coeffs section in the specified file.
    
    Args:
    - input_file_path (str): Path to the input file.
    - output_file_path (str): Path to the output file.
    - LJ1s, LJ2s: PairIJ Coeffs section.
    - Masses: The mass section
    """
    # Read the content of the file
    with open(input_file_path, 'r') as file:
        content = file.read()

    # Replace "9 atom types" with "10 atom types"
    content = content.replace("9 atom types", "10 atom types")
    
    # Define the regex pattern to find the section to replace
    pattern = re.compile(r'(PairIJ Coeffs # lj/charmm/coul/long\n)(.*?)(\nBond Coeffs # harmonic)', re.DOTALL)
    
    # Pair LJ section
    new_pairij_coeffs = "\n"
    for atomID, LJ1, LJ2 in zip(atomIDs, LJ1s, LJ2s):
        new_pairij_coeffs += f"{atomID} {LJ1:.10f} {LJ2:.10f}\n"
        
    # Mass section 
    new_masses = "\n"
    for atomID, mass in zip(atomIDs,Masses):
    	new_masses += f"{atomID} {mass:.4f}\n"

    # Replace the old PairIJ Coeffs section with the new content
    new_content = pattern.sub(r'\1' + new_pairij_coeffs + r'\3', content)
    
    # Define the regex pattern to find the section to replace
    pattern = re.compile(r'(Masses\n)(.*?)(\nPairIJ Coeffs # lj/charmm/coul/long)', re.DOTALL)

    # Replace the old PairIJ Coeffs section with the new content
    new_content = pattern.sub(r'\1' + new_masses + r'\3', new_content)
    
    # replace the pair
    new_content = new_content.replace("PairIJ Coeffs # lj/charmm/coul/long", "Pair Coeffs # lj/cut/coul/long")

    # Write the updated content back to the file
    with open(output_file_path, 'w') as file:
        file.write(new_content)

    print("File updated successfully.")



# Command line arguments
alpha = float(sys.argv[1])
inputfile = sys.argv[2]
outputfile = sys.argv[3]

# Define the input and output file paths
input_file_path = inputfile
output_file_path = outputfile

# Define the new content for the PairIJ Coeffs section
atomIDs = [1,2,3,4,5,6,7,8,9,10]
LJ1s = [0.0699984928,0.1299984877,0.2800057201,0.0800014955,0.0000000000,0.1521000000,0.0460000000,0.0469000000,0.1500000000,0.0150000000]
LJ2s = [3.0290604000,2.9399690301,3.8308580628,3.2963200383,0.0000000000,3.1507000000,0.2245000000,2.5136000000,4.0445000000,2.1114299620]
Masses = [15.9994,40.08,30.9738,15.9994,1.008,15.9994,1.008,22.98977,35.453,24.305]


LJ1L = LJ1s[1] + alpha*(LJ1s[-1]-LJ1s[1])
LJ2L = LJ2s[1] + alpha*(LJ2s[-1]-LJ2s[1])
MassesL = Masses[1] + alpha*(Masses[-1]-Masses[1])

LJ1s[-1] = LJ1L
LJ2s[-1] = LJ2L
Masses[-1] = MassesL


# Modify the PairIJ Coeffs section in the file
modify_coeffs(input_file_path, output_file_path, LJ1s, LJ2s, Masses)

