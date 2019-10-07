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

	-wipe-existing-data=* | --wipe-existing-data=* )
	    WIPEEXISTING=`expr "x$option" : "x-*wipe-existing-data=\(.*\)"`
	    ;;

	-trilinos-pfx=* | --trilinos-pfx=* )
	    TRILINOSPFX=`expr "x$option" : "x-*trilinos-pfx=\(.*\)"`
	    ;;

	-do=* | --do=* )
	    WHICHTASK=`expr "x$option" : "x-*do=\(.*\)"`
	    ;;

	-dense-jac=* | --dense-jac=* )
	    DENSEJACOBIAN=`expr "x$option" : "x-*dense-jac=\(.*\)"`
	    ;;

	-dbg-print=* | --dbg-print=* )
	    WITHDBGPRINT=`expr "x$option" : "x-*dbg-print=\(.*\)"`
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

Configuration:
-h, --help				display help and exit

--working-dir=				the working directory where we build/run

--with-env-script=<path-to-file>	full path to script to set the environment.
					default = assumes environment is set.

--trilinos-pfx=				full path to installation of Trilinos
					If not provided, I will build it

--wipe-existing-data=[yes/no]		if yes, all the following subfolders:
						--target-dir/data_*
						--target-dir/build_*
					will be fully wiped and re-made.
				   	default = no

--do=					which case/stage to execute

--dense-jac=[yes/no]			if yes, use dense jacobian
					default = no

--dbg-print=[yes/no]			if yes, enable pressio debug print
					default = no
EOF
  exit 0
fi
