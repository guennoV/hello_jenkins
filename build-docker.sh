#!/bin/bash
set -e

script_dir=$(cd `dirname "${BASH_SOURCE[0]}"` && pwd)

docker build $* -t uavia-corekit $script_dir
