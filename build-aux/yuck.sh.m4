## -*- shell-script -*-
changequote`'changequote([,])dnl
changecom([#])dnl

usage()
{
	cat <<'EOF'
Usage: YUCK_UMB_STR [[OPTION]]...dnl
ifelse(yuck_cmds(), [], [], [ COMMAND])[]dnl
ifelse(defn([YUCK_UMB_POSARG]), [], [], [ defn([YUCK_UMB_POSARG])])
ifelse(yuck_umb_desc(), [], [], [dnl
yuck_umb_desc()
])dnl
EOF
}

usage
