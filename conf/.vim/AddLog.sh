#----------------configure your log info----------------#
LOG_PATH=./
LOG=$LOG_PATH/template.log.`date +%Y_%m_%d`
#----------------configure your rsync info--------------#
RSYNC_RETRY_TIMES=10
RSYNC_SLEEP=10
#---------------write your usage function---------------#
function usage()
{
	echo "";
	echo "usage: $1 [-h] [-l loglevel]";
	echo "Options:";
	echo -e "\t-h\tshow help message.";
	echo -e "\t-l\tset the log level:";
	echo -e "\t  \t\t0 --- DEBUG <default>";
	echo -e "\t  \t\t1 --- INFO";
	echo -e "\t  \t\t2 --- WARN";
	echo -e "\t  \t\t3 --- ERROR";
	echo -e "\t  \t\t4 --- FATAL";
	echo "template shell script";
	echo "";
}
#---------------------------auto generate---------------------------#
#{{{
LOG_LEVEL=0
LOG_STR=("DEBUG" "INFO" "WARN" "ERROR" "FATAL")
function log()
{

	logLevel=$1;
	logMsg="$2";
	if [ ! -z "`echo ${LOG_STR[*]} | grep $logLevel`" ]; then	
		echo "["`date +"%Y-%m-%d %H:%M:%S"`"] [$logLevel]:$logMsg" >> $LOG 2>&1;
	fi
}
function common_rsync()
{
	SSH_OPTION="/usr/bin/ssh -2 -q -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null ";
	RSYNC_CMD="/usr/bin/rsync";
	$RSYNC_CMD -auz -e "$SSH_OPTION" $1 $2 ;
	count=1;
	while [ $? -ne 0 ]; do
		if [ $count -ge $RSYNC_RETRY_TIMES ]; then
			return 1;
		fi
		sleep 10;
		count=`expr $count + 1 `;
		$RSYNC_CMD -auz -e "$SSH_OPTION" $1 $2 ;
	done
	return 0;
}
function get_conf()
{
	check=`grep "^$2=" $1`;
	if [ $? -eq 0 ]; then
		echo "$check"| head -n 1 | awk -F"=" '{print $2}'
	else
		echo $3;
	fi
}

while getopts "hl:" opt
do
	case $opt in
		h)usage $0;
		exit 2;;
		l)
		LOG_LEVEL="$OPTARG";
		if [[ ! "$OPTARG" =~ "^[[:digit:]]*$" ]]; then 
			usage $0; exit 2;
		fi
		if [ $OPTARG -lt 0 -o $OPTARG -gt 4 ]; then 
			usage $0; exit 2;
		fi
		LOG_STR=(${LOG_STR[*]:$OPTARG:4})
		;;
		?)usage $0; exit 2;;
	esac
done

if [ ! -d $LOG_PATH ] ;then 
	mkdir -p $LOG_PATH ;
fi

if [ ! -e $LOG ] ; then
	touch $LOG;
fi
#}}}
#--------------------------your code here---------------------------#
#    use like [ log "ERROR" "this is error message!";             ] #
#    use like [ common_rsync SRC DEST;                	          ] #
#    use like [ val=`get_conf "CONF_FILE" "KEY" "DEFAULT_VALUE"`; ] #
#-------------------------------------------------------------------#
