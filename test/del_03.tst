## -*- shell-script -*-

$ rotz add pg pic_03.jpeg pic_04.jpeg
$ rotz del -v <<EOF
pg
EOF
-pg	pic_03.jpeg
-pg	pic_04.jpeg
$

## del_03.tst ends here
