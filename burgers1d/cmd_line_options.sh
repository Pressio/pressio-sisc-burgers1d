#!/bin/bash

echo ""
echo "--------------------------------------------"
echo "**parsing cline arguments**"
echo ""

for option; do
    echo $option
    case $option in
	-help | --help | -h)
	    want_help=yes
	    ;;

	-working-dir=* | --working-dir=* )
	    WORKINGDIR=`expr "x$option" : "x-*working-dir=\(.*\)"`
	    ;;

	-with-env-script=* | --with-env-script=* )
	    SETENVscript=`expr "x$option" : "x-*with-env-script=\(.*\)"`
	    ;;

	-wipe-existing=* | --wipe-existing=* )
	    WIPEEXISTING=`expr "x$option" : "x-*wipe-existing=\(.*\)"`
	    ;;

	-do=* | --do=* )
	    WHICHTASK=`expr "x$option" : "x-*do=\(.*\)"`
	    ;;

	# unrecognized option}
	-*)
	    { echo "error: unrecognized option: $option
	  Try \`$0 --help' for more information." >&2
	      { (exit 1); exit 1; }; }
	    ;;

    esac
done


if test "$want_help" = yes; then
  cat <<EOF
\`./main_tpls.sh' build tpls

Usage: $0 [OPTION]...

Defaults for the options are specified in brackets.

Configuration:
-h, --help				display help and exit

--working-dir=				the working directory where we build/run

--with-env-script=<path-to-file>	full path to script to set the environment.
					default = assumes environment is set.

--wipe-existing=[0/1]			if =1 (true), the build and installation subdirectories of the
					destination folder set by --target-dir will be wiped and remade.
					default = 1.

--do=					which case/stage to execute
					options=build, fomTiming, fomBasis, rom

EOF
fi

# if help, then exit
test -n "$want_help" && exit 0
