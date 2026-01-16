. ./env.sh
./clean.sh
./CFGraph /home/projects/snort-2.9.13/snort_O3 /home/fix/snort/snort_O3_fix_GenerateSnortEventRtn GenerateSnortEventRtn
./clean.sh

./CFGraph /home/projects/snort-2.9.13/snort_O3 /home/fix/snort/snort_O3_fix_ServiceMapAddOtnRaw ServiceMapAddOtnRaw
./clean.sh

./CFGraph /home/projects/snort-2.9.13/snort_O3 /home/fix/snort/snort_O3_fix_PQ_Single PQ_Single
./clean.sh


