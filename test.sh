#!/bin/bash

function build(){
	path="$1"
	run=1
	local i=0
	echo "start-time:`date +%s`" > ./data
	while ((i++ <= 10));do
		echo "i:$i" >./data
		echo -n  "stat:"  >./data
		case $i in
			0)
				echo "update $path" >./data
				;;
			3)
				echo "build $path" >./data
				;;
			9)
				echo "tar $path" >./data
				;;
		esac
		sleep 2
	done

	echo "stat:end" >./data
	echo "end-time:`date +%s`" >./data
	run=0
	kill -10 $run_uid
}

function recv_build(){
#	wait $run_pid
	run_pid=0
	cur_path=
}

trap "recv_build"  SIGUSR1
[ ! -p "./data" ] && mkfifo data

run_pid=0
run_uid=$$
cur_path=

#(
#while true ;do
#	wait $run_uid
#done > data
#)&
#pid_dae=$!
#echo "pid:$pid_dae"
echo ">>> $$";
echo ">>> $UID"

. ./build_client.config


while read str ;do
	arv0=`echo "$str"|cut -d ':' -f 1`
	arv1=`echo "$str"|cut -d ':' -f 2`
	echo "sh_recv:$str" >&2
	case "$arv0" in
		"start-run")
			echo "name:$name"
			;;
		"test")
			./echo.sh
			;;
		"exit")
			[ $run_pid != 0 ] && kill -9 $run_pid
			echo "exit" 
			break
			;;
		"start_build") 
			if [ $run_pid != 0 ];then
				echo "error:areadly build" 
				continue
			else
				(
				function e(){
					echo xxx;
				}
				trap "e"  SIGUSR1

				build "$arv1") >&2  &
				run_pid=$!
				cur_path="$arv1"
				echo "success:$arv1"
				echo "pid:$run_pid" >&2
			fi
			;;
		"stop")
			if [ $run_pid != 0 ];then 
				kill -9 $run_pid
				run_pid=0
				cur_path=
				echo "stop:success" >&2
			else
				echo "stop:no run" >&2
			fi
			;;
		"path")
			echo "ptah:$cur_path" >&2
			;;
		"run-kill")
			if [ $run_pid != 0 ];then
				kill -10 $run_pid
			fi
			;;
		"pid_dae")
			echo "pid_dae:$pid_dae" >&2
			;;
		"pid")
			echo "pid:$run_pid" >&2
			;;
		"uid")
			echo "uid:$run_uid" >&2
			;;
		*)
			echo "$str" > ./data
	esac

	echo -n ">:" >&2
done > ./data
main_pid=$!
echo "main_pid:$main_pid"

#kill -9 $pid_dae
wait $pid_dae

