TAppEncoder.exe -c cfg\%1.cfg -q %2 -b str/%1_Q%2_S%3_H%4.bin -o enc.yuv -f %5 -g 1 -1 LDC -rg 4 -r 4 -rb0 2 -rb1 2 -ldm 1 -s %3 -h %4 -q %2 -%6 GPB  -ip -1 > log/enc_%1_Q%2_S%3_H%4.log
TAppDecoder.exe -b str/%1_Q%2_S%3_H%4.bin -o dec.yuv > log/dec_%1_Q%2_S%3_H%4.log
fc /b dec.yuv enc.yuv > log/fc_%1_Q%2_S%3_H%4.log