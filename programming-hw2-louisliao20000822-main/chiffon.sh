#!/bin/bash



declare -a score 
declare -a ava_host
declare -i free_host
free_host=$host
declare -a player_rank
declare -i max
declare -i cur_rank
declare -i top_score
declare -i max_player

while getopts “m:l:n:?” argv
do
     case $argv in
         m)
             host=$OPTARG
             ;;
         l)
             lucky_num=$OPTARG
             ;;
         n)
             player=$OPTARG
             ;;
     esac
done




max=0
free_host=$host


function cau_rank(){
	cur_rank=0
	top_score=100000000
	for ((i=1 ; i<=$player ; i=i+1));do
		max=0
		for ((j=1 ; j<=$player ; j=j+1));do
			if [ ${score[$j]} -ge $max ] && [ ${player_rank[$j]} -eq 0 ];then
				max=${score[$j]}
				max_player=$j
			fi;
		done;
		if [ $max -lt $top_score ];then
		top_score=$max
		cur_rank=$i
		fi;
		player_rank[$max_player]=$cur_rank
	done;	
	for((i=1 ; i<=$player ; i=i+1));do
		echo "$i ${player_rank[$i]}"
	done;	

}



mkfifo "fifo_0.tmp"
exec {fd}<> fifo_0.tmp

for((p=0 ; p<=$player ; p=p+1));do
	score[$p]=0
	player_rank[$p]=0
done;	

for((i =1 ; i <=$host ;i=i+1));do
	mkfifo "fifo_$i.tmp"
	exec {fd}<> fifo_${i}.tmp
	ava_host[$i]=1
done;

for((i=1;i<=host;i=i+1));do
	./host -m $i -d 0 -l $lucky_num &
done;


for ((a=1 ; a<=$player ; a=a+1)); do
for ((b=$a+1 ; b<=$player ; b=b+1)); do
for ((c=$b+1 ; c<=$player ; c=c+1)); do
for ((d=$c+1 ; d<=$player ; d=d+1)); do
for ((e=$d+1 ; e<=$player ; e=e+1)); do
for ((f=$e+1 ; f<=$player ; f=f+1)); do
for ((g=$f+1 ; g<=$player ; g=g+1)); do
for ((h=$g+1 ; h<=$player ; h=h+1)); do
while [ $free_host -eq 0 ]; do
    read host_id < "fifo_0.tmp"
    for ((p=0 ; p<8 ; p=p+1));do
        read id player_score < "fifo_0.tmp"
        ((score[$id]+=$player_score))
    done;
    ((free_host++))
    ava_host[$host_id]=1
done;
for ((i=1 ; i<=$host ; i=i+1));do
    if [ ${ava_host[$i]} -eq 1 ];then
        ava_host[$i]=0
        echo "$a $b $c $d $e $f $g $h" > "fifo_$i.tmp"
        ((free_host--))
        break
    fi;
done;
done;done;done;done;done;done;done;done;


while [ $free_host != $host ]; do
    ((free_host++))
    read host_id < "fifo_0.tmp"   
    for ((p=0 ; p<8 ; p=p+1));do
        read id player_score < "fifo_0.tmp"
        ((score[$id]+=$player_score))
    done;
    ava_host[$host_id]=1
done;


for ((i=1; i<=$host;i++));do

	echo "-1 -1 -1 -1 -1 -1 -1 -1" > "fifo_$i.tmp"
done;

cau_rank

for ((i=1; i<=$host;i++));do
	exec {fd}<&-
done


for ((i=0; i<=$host;i++));do
	rm "fifo_$i.tmp"
done

wait
