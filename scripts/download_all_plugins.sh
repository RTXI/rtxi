#!/bin/bash

set -eu

if [ ! -d all_plugins ]; then
    mkdir all_plugins
    cd all_plugins/
else
    cd all_plugins
fi

REPO_PREFIX="git@github.com:RTXI/"
declare -a EXCLUDE_LIST=( 
    "${REPO_PREFIX}rtxi.git" 
    "${REPO_PREFIX}rtxi.github.io.git" 
    "${REPO_PREFIX}genicam-camera.git" 
    "${REPO_PREFIX}rtxi-crawler.git" 
    "${REPO_PREFIX}matlab-tools.git" 
    "${REPO_PREFIX}tutorials.git" 
    "${REPO_PREFIX}autapse.git" 
    "${REPO_PREFIX}camera-control.git" 
    "${REPO_PREFIX}gen-net.git" 
    "${REPO_PREFIX}dynamo-examples.git" 
    "${REPO_PREFIX}plot-lib.git" 
    "${REPO_PREFIX}python-plugin.git" 
    "${REPO_PREFIX}poster.git" 
    "${REPO_PREFIX}user-manual.git" 
    "${REPO_PREFIX}logos.git" 
    "${REPO_PREFIX}live-image.git" 
    "${REPO_PREFIX}conference-2015.git" 
)

REPO_LIST=(`curl https://api.github.com/orgs/rtxi/repos?per_page=100 | grep -o "${REPO_PREFIX}.*\.git"`)

for repo in ${REPO_LIST[@]}; do
    if [[ ! "${EXCLUDE_LIST[*]}" =~ "${repo}" ]]; then
        echo "Cloning repo: $repo"
        git clone ${repo}
    fi
done

