#!/bin/bash

#file=/home/cfmcginn/Samples/HYDJET/hydjet_MERGED_SVM_20190223.root
file=/home/cfmcginn/Samples/Forest/PYTHIA6_HYDJET/HiForestAOD_PbPb_MCDijet30_20180218_ExcludeTop4_ExcludeToFrac_Frac0p7_Full_5Sigma_MERGED_SKIM_20190702.root
config=/home/cfmcginn/Projects/RandomCone/configs/simpleConfigTEMPLATE.txt

DATE=`date +%Y%m%d`

mkdir -p output/$DATE

rParams=(0.2 0.3 0.4 0.6 0.8 1.0)
#rParams=(0.4)
ptCuts=(0.0)

for i in "${rParams[@]}"
do   
    for j in "${ptCuts[@]}"
    do
	rParam=$(echo $i | sed -e "s@\\.@p@g")
	ptCut=$(echo $j | sed -e "s@\\.@p@g")
	newFile=/home/cfmcginn/Projects/RandomCone/output/$DATE/simpleConfig_"R$rParam"_"PTCUT$ptCut".txt
	cp $config $newFile

	sed -i -e "s@PTCUTVAL@$j@g" $newFile
	sed -i -e "s@RCRVAL@$i@g" $newFile
	
#	./bin/processForestForRC.exe $file $newFile
    done
done


for i in output/$DATE/*RCForest_$DATE.root
do
    ./bin/makeHistRC.exe $i
done

for i in output/$DATE/*HISTRC_$DATE.root
do
    ./bin/plotHistRC.exe $i
done

