#!/bin/bash

project_path="$(realpath $1)"
patch_file="$(realpath $2)"
current_dir="$(pwd)"

cd $project_path
# If the git repo has not been modified, then apply patch
if git diff-index --quiet HEAD --; then
    git apply $patch_file
fi
cd $current_dir