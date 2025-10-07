#include "random_walk.h"

int main(){
    
    RandomWalk RW;
    RW.initialize();
    RW.initialize_properties();

    for(int i{}; i < RW.get_nblocks(); i++){
        for(int j{}; j < RW.get_nwalksperblock(); j++){           
            for(int k{}; k < RW.get_nstep(); k++){
                RW.step();
                RW.measure(k);
                if(j == RW.get_RWprint()){
                    RW.write_RW(k, j, i);
                }
            }
            RW.write_last_position(j+i*RW.get_nwalksperblock());
            RW.reset_position();
        }
        RW.averages(i);
        RW.reset_block(i);
    }

    RW.finalize();

    return 0;

}
