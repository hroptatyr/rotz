## -*- shell-script -*-

$ rm -f -- rotz.tcb
$ rotz add xxx pic_01.jpeg pic_02.jpeg
$ rotz add clean pic_03.jpeg pic_04.jpeg
$ rotz add sun pic_02.jpeg pic_03.jpeg
$ rotz show sun clean
pic_02.jpeg
pic_03.jpeg
pic_03.jpeg
pic_04.jpeg
$ rm -f -- rotz.tcb

## show_01.tst ends here
