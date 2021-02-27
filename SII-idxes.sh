#!/bin/sh
# @file: SII-idxes.sh
# @brief: compute SII indices
# $1 - print only one specified signal (pass name)

DH="PUPG/include/database.h"

# names of start of all sections
SIGNALSII_STARTS="
SIGNALSII_SENSORSSTAT_START
SIGNALSII_INSTSENSORSSTAT_START
SIGNALSII_MODCMD_START
SIGNALSII_BURNER_START
SIGNALSII_OUTSIG_START
SIGNALSII_TEMPPRM_START
SIGNALSII_GSM_START
SIGNALSII_STATISTIC_START
SIGNALSII_SERVICE_START
SIGNALSII_FUELCONS_START
SIGNALSII_OTHER_START
"
# names of sensors
SENSORS_LIST="
Sensor_Supply
Sensor_Return
Sensor_Room
Sensor_Outside
Sensor_Tray
"
# names of sensors positions
SENSORS_VAL_LIST="
DSW_SENSOR_SUPPLY_POSITION
DSW_SENSOR_RETURN_POSITION
DSW_SENSOR_ROOM_POSITION
DSW_SENSOR_OUTSIDE_POSITION
DSW_SENSOR_TRAY_POSITION
"
# end of SII defines
SIGNALSII_END="#define SII_SIZE"

# **************************************************************************************************************

# get defined value $1 from file $2; ex: '#define VAL 0'
get_val() {
	grep "#define $1" $2 | sed -E "s/[[:space:]]+/:/g" | cut -d: -f3 | tr -d '\r'
}

# **************************************************************************************************************

tfn="._start_size_tmp"
# read start / size
cnt=0
for s in $SIGNALSII_STARTS; do
	ss=$( echo $s | sed "s/_START/_SIZE/" )
	if [ $cnt -eq 0 ]; then
		echo "$s=\$( get_val $s $DH )" > $tfn
		echo "$ss=\$( get_val $ss $DH )" >> $tfn
	else
		echo "$s=\$( get_val $s $DH | sed \"s/)// ; s/(// ; s/$prev_s/\$$prev_s/ ; s/$prev_ss/\$$prev_ss/\" | bc -l )" >> $tfn
		echo "$ss=\$( get_val $ss $DH )" >> $tfn			
	fi
	prev_s=$s
	prev_ss=$ss
	cnt=$(($cnt+1))
done
# read sensors index
cnt=0
svl=($SENSORS_VAL_LIST)
for s in $SENSORS_LIST; do
	echo "$s=\$( get_val ${svl[$cnt]} $DH )" >> $tfn
	cnt=$(($cnt+1))
done

# **************************************************************************************************************

. $tfn

sii_strts=($SIGNALSII_STARTS)
old_IFS=$IFS
IFS=]
VAR_LIST=( $( sed -n -E "/#define ${sii_strts[0]}/,/$SIGNALSII_END/ { /[\[]/ { s/[[:space:]]+/ /g ; s/ *\+ */\+/g ; s/#define // ; s/\].*/\]/ ; s/ .*DB->Signals_intI\[/ \[/ p  } }" $DH | tr -d '\r' ) )
IFS=$old_IFS

VAR_NAME_LIST=
echo "" > $tfn
for v in "${VAR_LIST[@]}"; do
	v=( $( echo $v | sed "s/\[//" ) )
	VAR_NAME_LIST="$VAR_NAME_LIST ${v[0]}"
	echo -n "${v[0]}=\$((" >> $tfn
	val=${v[1]}
	# insert $ before all words
	val=$( echo $val | sed -E "s/^([A-Za-z_]+)/$\1/ ; s/\+([A-Za-z_]+\+)/\+$\1/g ; s/\+([A-Za-z_]+)/\+$\1/" )
	echo "$val))" >> $tfn
done

# **************************************************************************************************************

. $tfn

alignprint () {
        echo -n "$1 "
        eap_len="$(echo -n "$1" | wc -m)"
        eap_dots=$((40-$eap_len))
        printf "%0.s-" $(seq 1 $eap_dots)
        if [ "$2" != "" ] ; then
                echo "$2"
        fi
}

if [ "$#" -ne 0 -a "$1" != "all" ]; then
	v=$1
	echo "SII $v index: " $(($v))
else
	echo "SII indeces:"
	for v in $VAR_NAME_LIST; do
		alignprint "	$v" $(($v))
	done
fi

rm -rf $tfn
exit 0


