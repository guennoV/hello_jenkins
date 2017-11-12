#!/bin/bash
set -e

script_dir=$(cd `dirname "${BASH_SOURCE[0]}"` && pwd)

autoreconf $script_dir/coreKit
