
# rldist.sh
#
# Purpose:	To build the RL02 distribution.
# Usage:	rldist.sh [path]
# Environment:	Bourne shell script
# Date:		3/27/84
# Author:	Alan Delorey
# 
# Remarks:
#    This script controls the process of building the RL02 distribution
#    kit.
#

# If there is a pathname given:
# then use it.  First we need to change the /'s to .'s so that sed can strip
#	off the pathname (since sed delimits exprs with / there can't be any
#	imbedded in the substitution string).
# else work with current directory as the starting point.

if test $1
then
    cat << EOF > tmp1_$$
$1
EOF
    sed '/\//s//./g' tmp1_$$ > tmp2_$$
    exec < tmp2_$$
    read mount

    cat << EOF > sed_tmp_$$
/^$mount/s///
EOF
#    find $1 -print | sed -f sed_tmp_$$
    find $1 -print | sed -f sed_tmp_$$ | getstat $1 > inv
    ifempty.sh $1 < dir_inv
    createmd < inv > makerldist
    chmod +x makerldist
#    cd $1
#    makerldist
    rm dir_inv tmp1_$$ tmp2_$$ sed_tmp_$$
else
    find -print | getstat > inv
    ifempty.sh
    createmd < inv > makerldist
    chmod +x makerldist
#    makerldist
fi
