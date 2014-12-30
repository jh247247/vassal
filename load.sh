dfu-suffix -a ./main.bin -v 1eaf -p 0003 -d 0001
dfu-util -d 1eaf:0003 -a 1 -D ./main.bin -R
