extern int value_parity(),level_driver_run();extern int current_slot,g9ade,g73e;
#include "C470.H"
void f4943(i) register int i;{slot_table[current_slot].byte12=i+1;g9ade=slot_table[current_slot].sub[i]*2+i*8;g73e=-1;while(1){if(value_parity(g9ade)){if(!level_driver_run())g9ade-=2;else{slot_table[current_slot].sub[i]++;if((g9ade&7)==7)break;}}else{while(!level_driver_run());}g9ade++;}}
