## -*- shell-script -*-

$ rm -f -- rotz.tcb
$ rotz add <<EOF
p1	b1
p1	b2
p2	b3
p3	b1
p3	b2
p3	b3
p4	b1
p4	b3
p4	b4
EOF
$ rotz show p1
b1
b2
$ rotz show b1
p1
p3
p4
$

## show_02.tst ends here
