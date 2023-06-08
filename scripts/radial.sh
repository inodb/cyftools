#!/bin/bash

#SBATCH -c 1                               # Request one core
#SBATCH -t 0-12:00                         # Runtime in D-HH:MM format
#SBATCH -p short                           # Partition to run in
#SBATCH --mem=4G                          # Memory total in MiB (for all cores)
#SBATCH -o hostname_%j.out                 # File to which STDOUT will be written, including job ID (%j)
#SBATCH -e hostname_%j.err                 # File to which STDERR will be written, including job ID (%j)

input_file=$1
output_file=$2

## parameters

# radial file
RAD=/home/jaw34/projects/orion/radial.csv

if [[ ! -f "$input_file" ]]; then
    echo "Error: File '$input_file' does not exist."
    exit 1
else
    #echo "... running: gunzip -c $input_file | cysift radialdens - - -f ${RAD} | cysift clean - -G $output_file    "
    #gunzip -c $input_file | cysift radialdens - - -f ${RAD} | cysift clean - -G $output_file
    echo "... running: cysift radialdens $input_file -f ${RAD} $output_file"
    cysift radialdens $input_file -f ${RAD} $output_file
fi
