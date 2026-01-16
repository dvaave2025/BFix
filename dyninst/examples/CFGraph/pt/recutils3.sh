. ./env.sh
./clean.sh
./CFGraph /home/projects/recutils-1.8/librec_O3.so.1.0.0 /home/fix/recutils/librec_O3_fix_rec_encrypt.so.1.0.0 rec_encrypt
./clean.sh

./CFGraph /home/projects/recutils-1.8/librec_O3.so.1.0.0 /home/fix/recutils/librec_O3_fix_rec_db_merge_records.so.1.0.0 rec_db_merge_records
./clean.sh

./CFGraph /home/projects/recutils-1.8/librec_O3.so.1.0.0 /home/fix/recutils/librec_O3_fix_rec_int_collect_field_list.so.1.0.0 rec_int_collect_field_list
./clean.sh

./CFGraph /home/projects/recutils-1.8/librec_O3.so.1.0.0 /home/fix/recutils/librec_O3_fix_rec_rset_group.so.1.0.0 rec_rset_group
./clean.sh
