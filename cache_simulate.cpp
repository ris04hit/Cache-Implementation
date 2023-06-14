#include "cache_simulate.hpp"

int main(int argc, char* argv[]){
	if (argc < 7){
		std::cerr << "Arguments Missing\n";
		return 0;
	}
    int block_size, L1_size, L2_size, L1_assoc, L2_assoc, L1_access, L2_acess, DRAM_access;
    try{
        block_size = stoi(argv[1]);
        L1_size = stoi(argv[2]);
        L1_assoc = stoi(argv[3]);
        L2_size = stoi(argv[4]);
        L2_assoc = stoi(argv[5]);
        L1_access = 1;
        L2_acess = 20;
        DRAM_access = 200;
    }
    catch(const std::exception& e){
        std::cerr << "Parameter not convertible to integer\n";
        return 0;
    }  
    for (int i = 0; i<argc-6; i++){
        string filename = argv[6+i];
        Cache_Simulator* cache_simulator;
        ifstream file(filename);
        if (file.is_open()){
            cout << "\nExecuting file " << filename << endl;
            cache_simulator = new Cache_Simulator(block_size, L1_size, L2_size, L1_assoc, L2_assoc, L1_access, L2_acess, DRAM_access, file);}
        else{
            cerr << "File could not be opened. Terminating...\n";
            return 0;
        }
        file.close();
        cache_simulator->run_simulation(); 
    }
	return 0;
}