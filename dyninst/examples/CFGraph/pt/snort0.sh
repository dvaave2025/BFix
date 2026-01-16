. ./env.sh
./clean.sh
./CFGraph /home/projects/snort-2.9.13/snort_O0 /home/fix/snort/snort_O0_fix_GenerateSnortEventRtn GenerateSnortEventRtn
./clean.sh

./CFGraph /home/projects/snort-2.9.13/snort_O0 /home/fix/snort/snort_O0_fix_ServiceMapAddOtnRaw ServiceMapAddOtnRaw
./clean.sh

./CFGraph /home/projects/snort-2.9.13/snort_O0 /home/fix/snort/snort_O0_fix_PQ_Single PQ_Single
./clean.sh


