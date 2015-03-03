#!/bin/sh

# Usage info
show_help() {
cat << EOF
Usage: ${0##*/} [-hv] [-f OUTFILE] [FILE]...
Do stuff with FILE and write the result to standard output. With no FILE
or when FILE is -, read standard input.

    -h          display this help and exit
    -f OUTFILE  write the result to OUTFILE instead of standard output.
    -v          verbose mode. Can be used multiple times for increased
                verbosity.
EOF
}

# Initialize our own variables:
output_file=""
verbose=0

OPTIND=1 # Reset is necessary if getopts was used previously in the script.  It is a good idea to make this local in a function.
while getopts "hvf:" opt; do
    case "$opt" in
        h)
            show_help
            exit 0
            ;;
        v)  verbose=$((verbose+1))
            ;;
        f)  output_file=$OPTARG
            ;;
        '?')
            show_help >&2
            exit 1
            ;;
    esac
done
shift "$((OPTIND-1))" # Shift off the options and optional --.

#printf 'verbose=<%d>\noutput_file=<%s>\nLeftovers:\n' "$verbose" "$output_file"
#printf '<%s>\n' "$@"
input=$1
strings $1 > tmp
sed -e 's/[^[[:digit:]]*//g' -e "s/\*//g" -e 's/\[//g' -e '/^[0-9]\{1,6\}$/d' -e '/^$/d' tmp | cut -c 1-8 > $output_file
wc -l $output_file
awk -F, '{ print $1 }' ~/miniature-ryu/client/data.txt > ~/miniature-ryu/client/client.txt
diff -u ~/miniature-ryu/client/client.txt $output_file | grep ^[+-].[0-9] | wc -l
#rm tmp
# End of file
