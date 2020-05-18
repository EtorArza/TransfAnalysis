#https://gist.github.com/fadookie/4a08de38784dfdaa1f31952c1792d4dd

# WARNING! The following is not as safe as I originally thought and I'm not sure yet the right way to do this.
# See https://unix.stackexchange.com/questions/383541/how-to-save-restore-all-shell-options-including-errexit

enable_safety() {
    BITRISE_CLI_PREVIOUS_SHELL_OPTIONS=$(set +o)
    set -o nounset
    set -o errexit
    set -o pipefail
}

disable_safety() {
    eval "$BITRISE_CLI_PREVIOUS_SHELL_OPTIONS"
    unset BITRISE_CLI_PREVIOUS_SHELL_OPTIONS
}



##
# Takes a pipe-delimited string and returns a bash array.
# Sets the array to global varaible BITRISE_CLI_LAST_PARSED_LIST
# Make a copy between calls so it doesn't get overwritten
# ex. list_to_array "some|list"
# list1=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
##
list_to_array () {
    enable_safety
    IFS='|' read -a BITRISE_CLI_LAST_PARSED_LIST <<< "$1"
    disable_safety
}

##
# Converts arguments (such as an array) to a pipe-delimited string
# ex. output_list=$(to_list "${list1[@]}")
##
to_list () {
    enable_safety
    local IFS='|'
    echo "$*"
    disable_safety
}


list_to_array_with_comma () {
    enable_safety
    IFS=',' read -a BITRISE_CLI_LAST_PARSED_LIST <<< "$1"
    disable_safety
}


to_list_with_comma () {
    enable_safety
    local IFS=','
    echo "$*"
    disable_safety
}


