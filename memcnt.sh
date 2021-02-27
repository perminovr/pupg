#!/bin/sh
# @file: memcnt.sh
# @brief: compute EEPROM mem: used/free

DH="PUPG/include/database.h"
DS="PUPG/src/database.c"
# max mem
PS=256
# service bytes
SERVI=$((1+2+1)) # cnt+crc+ver

# **************************************************************************************************************

# get defined value $1 from file $2; ex: '#define VAL 0'
get_val() {
	grep "#define $1" $2 | sed -E "s/[[:space:]]+/:/g" | cut -d: -f3 | tr -d '\r'
}

# **************************************************************************************************************
# compute PIH size via max values defined in c-file
PIH_size=
MAXVALS=$( sed -n "/_database_SetMinMaxValues_intMax_Start/,/_database_SetMinMaxValues_intMax_End/ {/_Max = /p}" $DS | sed -E "s/[[:space:]]+/:/g" | cut -d: -f4 | tr -d '\r' |  tr -d ';' | sed "s/AlgPrm_TTarget/99/" )

res=0
cnt=0
for v in $MAXVALS; do
	if [ $v -gt 255 ]; then
		res=$(($res+4))	
	else
		res=$(($res+1))	
	fi	
	cnt=$(($cnt+1))
done
# **************************************************************************************************************
ERRJ_size=$( get_val "ERRORS_JOURNAL_SIZE" $DH )
SIE1_size=$( get_val "Signals_intIE_1BYTE_SIZE" $DH | sed "s/)// ; s/(// ; s/ERRORS_JOURNAL_SIZE/$ERRJ_size/" | bc -l )
SIE4_size=$( get_val "Signals_intIE_4BYTE_SIZE" $DH )
# **************************************************************************************************************
GSMIN_size=$( get_val "GSM_NUMBERS_IN_SIZE" $DH )
GSMOUT_size=$( get_val "GSM_NUMBERS_OUT_SIZE" $DH )
GSMNUMS_size=$( get_val "GSM_TOTAL_NUMBERS" $DH | sed "s/)// ; s/(// ; s/GSM_NUMBERS_IN_SIZE/$GSMIN_size/ ; s/GSM_NUMBERS_OUT_SIZE/$GSMOUT_size/" | bc -l )
PCH_size=$( get_val "PCH_SIZE" $DH | sed "s/)// ; s/(// ; s/GSM_TOTAL_NUMBERS/$GSMNUMS_size/" | bc -l )
# **************************************************************************************************************
PFH_size=$( get_val "PFH_SIZE" $DH )
PCI_size=$( get_val "PCI_SIZE" $DH )
# **************************************************************************************************************

echo "EEPROM database:"
total=0
total=$(($total+$res))
echo "PIH_size"
echo "	cnt=$cnt"
echo "	bytes=$res"
#
cnt=$(($SIE1_size+$SIE4_size))
res=$(($SIE1_size+$SIE4_size*4))
total=$(($total+$res))
echo "SIE_size"
echo "	cnt=$cnt"
echo "	bytes=$res"
#
cnt=$PFH_size
res=$((PFH_size*4))
total=$(($total+$res))
echo "PFH_size"
echo "	cnt=$cnt"
echo "	bytes=$res"
#
cnt=$PCH_size
res=$((PCH_size*10-5)) # prefix only 5
total=$(($total+$res))
echo "PCH_size"
echo "	cnt=$cnt"
echo "	bytes=$res"
#
cnt=$PCI_size
res=$((PCI_size*12))
total=$(($total+$res))
echo "PCI_size"
echo "	cnt=$cnt"
echo "	bytes=$res"
#
echo
echo "--->	total bytes=$total"
freeb=$(($PS-$total-$SERVI))
echo "	free bytes=$freeb"


