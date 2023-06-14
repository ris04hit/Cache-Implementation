#ifndef __CACHE_SIMULATOR_HPP__
#define __CACHE_SIMULATOR_HPP__

#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

struct Cache_Simulator{
    struct Cache_Level{
        struct Set{
            struct Block{
                int valid_bit, dirty_bit, block_size, lru;
                long long tag;
                // Constructor to initialize Block
                Block(int block_size_, long long tag_, int lru_){
                    block_size = block_size_;
                    tag = tag_;
                    valid_bit = 0;
                    dirty_bit = 0;
                    lru = lru_;
                }
            }* set;
            int block_num, set_size, offset;
            // Constructor to initialize Set
            Set(int set_size_, int block_size, int set_index){
                set_size = set_size_;
                block_num = set_size/block_size;
                set = (Block*)malloc(sizeof(Block)*block_num);
                for (int i = 0; i < block_num; i++){
                    set[i] = Block(block_size, 0LL, set_index);
                }
                offset = 0;
                for(int size = 1; size < block_size; size <<= 1) offset++;
            }
        }* cache;
        int cache_size, set_num, read, read_miss, write, write_miss, index_count, memory_access_time, write_back_count; // index stores till which bit from right is included in index
        // Constructor to initialize Cache_Level
        Cache_Level(int size, int assoc, int block_size, int memory_access_time_){
            cache_size = size;
            set_num = assoc;
            cache = (Set*)malloc(sizeof(Set)*set_num);
            for (int i=0; i < set_num; i++){
                cache[i] = Set(size/set_num, block_size, i);
            }
            read = 0;
            read_miss = 0;
            write = 0;
            write_miss = 0;
            write_back_count = 0;
            index_count = 0;
            for (int size = 1; size < cache->set_size; size <<= 1) index_count++;
            memory_access_time = memory_access_time_;
        }

        // Prints cache parameters
        void print_cache(string name){
            if (read + write){
                cout << "Number of " << name << " reads:\t\t\t" << read << endl;
                cout << "Number of " << name << " read misses:\t\t" << read_miss << endl;
                cout << "Number of " << name << " writes:\t\t\t" << write << endl;
                cout << "Number of " << name << " write misses\t\t" << write_miss << endl;
                cout << name << " miss rate:\t\t\t\t" <<  ((read_miss + write_miss) * 1.0)/(read + write) << endl;
                cout << "Number of writebacks from " << name << " memory:\t" << write_back_count << endl;
            }
            else {
                cout << "No Requests yet" << endl;
            }
        }

        // Checks existence of block in cache
        int check_block(long long address){
            // cout << "Running check_bool for address = " << address << endl;
            int hit;
            long long tag, index;
            index = (((1<<index_count)-1) & address) >> cache->offset;
            tag = address >> index_count;
            hit = 0;
            for (int i = 0; i < set_num; i++){     // Checking hit in L
                // cout << "index = " << index << "\tset_index = " << i << "\ttag_out = " << tag << "\tvalid_bit = " << cache[i].set[index].valid_bit << "\ttag_in = " << cache[i].set[index].tag << endl;
                if (cache[i].set[index].valid_bit && (cache[i].set[index].tag == tag)){
                    hit = 1;
                }
            }
            return hit;
        }    

        // Inserts non-existent block in cache
        void insert_block(int dirty_bit, long long address){
            long long tag, index;
            Set::Block* block;
            index = (((1<<index_count)-1) & address) >> cache->offset;
            tag = address >> index_count;
            // cout << "Inserting block for address = " << address << "\tindex = " << index << "\ttag = " << tag << endl;
            for (int i = 0; i < set_num; i++){     // Updating new block in L
                block = &cache[i].set[index];
                if (block->lru == set_num - 1){
                    if (block->dirty_bit && block->valid_bit){
                        // cout << "writeback of block at address = " << address << endl;
                        write_back_count += 1;
                    }
                    block->dirty_bit = dirty_bit;
                    block->valid_bit = 1;
                    block->lru = set_num-1;
                    block->tag = tag;
                }
                // else{
                //     block->lru += 1;
                // }
                // cout << "set_index = " << i << "\tdirty_bit = " << block->dirty_bit << "\tvalid_bit = " << block->valid_bit << "\tlru = " << block->lru << "\ttag = " << block->tag << endl;
            }
        }

        void update_lru(long long address){
            long long tag, index;
            Set::Block* block;
            int lru;
            index = (((1<<index_count)-1) & address) >> cache->offset;
            tag = address >> index_count;
            // cout << "Updating lru of block for address = " << address << "\tindex = " << index << "\ttag = " << tag << endl;
            for (int i = 0; i < set_num; i++){     // Updating lru for block in L
                block = &cache[i].set[index];
                if (block->tag == tag){
                    lru = block->lru;
                }
            }
            for (int i = 0; i < set_num; i++){     // Updating lru for block in L
                block = &cache[i].set[index];
                if (block->lru == lru){
                    block->lru = 0;
                }
                else if (lru > block->lru){
                    block->lru += 1;
                }
                // cout << "set_index = " << i << "\tdirty_bit = " << block->dirty_bit << "\tvalid_bit = " << block->valid_bit << "\tlru = " << block->lru << "\ttag = " << block->tag << endl;
            }
        }

        // Returns writeback address + 1 if writeback required otherwise 0
        long long check_writeback(long long address){
            long long tag, index;
            Set::Block* block;
            long long writeback = 0;
            index = (((1<<index_count)-1) & address) >> cache->offset;
            tag = address >> index_count;
            for (int i = 0; i < set_num; i++){     // Updating new block in L
                block = &cache[i].set[index];
                if (block->lru == set_num - 1){
                    if (block->dirty_bit && block->valid_bit){
                        // cout << "Check writeback of block at address = " << address << endl;
                        writeback = (block->tag<<index_count) + (index<<cache->offset) + 1;
                    }
                }
                // cout << "set_index = " << i << "\tdirty_bit = " << block->dirty_bit << "\tvalid_bit = " << block->valid_bit << "\tlru = " << block->lru << "\ttag = " << block->tag << "\twriteback = " << writeback << endl;
            }
            return writeback;
        }

        // Sets dirty bit of block in cache
        void set_dirty_bit(long long address){
            long long tag, index;
            Set::Block* block;
            index = (((1<<index_count)-1) & address) >> cache->offset;
            tag = address >> index_count;
            for (int i = 0; i < set_num; i++){
                if (cache[i].set[index].valid_bit && (cache[i].set[index].tag == tag)){
                    block = &cache[i].set[index];
                    block->dirty_bit = 1;
                    // cout << "Setting Dirty Bit of block with address = " << address << endl;
                    // cout << "set_index = " << i << "\tdirty_bit = " << block->dirty_bit << "\tvalid_bit = " << block->valid_bit << "\tlru = " << block->lru << "\ttag = " << block->tag << endl;
                }
            }
        }

    }*L1,*L2;
    vector<pair<int,long long>> instructions;
    int dram_access_time;
    long long total_access_time = 0;

    // Constructor to initialize Cache_Simulator
    Cache_Simulator(int block_size, int L1_size, int L2_size, int L1_assoc, int L2_assoc, int L1_access_time, int L2_access_time, int DRAM_access_time, ifstream& file){
        L1 = new Cache_Level(L1_size, L1_assoc, block_size, L1_access_time);
        L2 = new Cache_Level(L2_size, L2_assoc, block_size, L2_access_time);
        dram_access_time = DRAM_access_time;
        parse(file);
    }

    // Parses the input file and inserts instruction in <instructions>
    void parse(ifstream& file){
        string line;
        int mode, line_num = 0;
        long long address;
        while(getline(file, line)){
            line_num++;
            // Parses one line and converts it to pair of <mode, address> where mode corresponds to 0 for read request and 1 for write request
            if (line[0] == 'r'){
                mode = 0;
            }
            else if(line[0] == 'w'){
                mode = 1;
            }
            else{
                cerr << "Invalid request mode in line " << line_num << endl;
                file.close();
                exit(1);
            }
            int index = 0, ascii;
            bool condition = false;
            while (index < line.length()-1 && (!condition)) {
                index++;
                ascii = static_cast<int>(line[index]);
                condition = (47<ascii && ascii<58) ||  (96<ascii && ascii<123);
            }
            try{
                address = stoll(&line[index], nullptr, 16);
                ;
            }
            catch(const std::exception& e){
                std::cerr << "Invalid Address in line " << line_num << endl;
                file.close();
                exit(1);
            }
            instructions.push_back(make_pair(mode, address));
        }
    }


    // Runs Simulation
    void run_simulation(){
        // cout << "Starting Simulation" << endl;
        int instruction_count = 0, mode, hit;
        long long address;
        while(instruction_count < instructions.size()){
            mode = instructions[instruction_count].first;
            address = instructions[instruction_count].second;
            // cout << "Instruction" << instruction_count << " " <<  mode << " " << address << endl;
            total_access_time += L1->memory_access_time;
            // cout << "Checks block existence in L1 \n";
            hit = L1->check_block(address);
            if (hit){     // Process when hits in L1
                // cout << "Hit in L1\n";
                if (mode){          // Write Instruction
                    L1->write += 1;
                    // cout << "Write Mode for hit in L1. New L1read_count = " << L1->read << "\tL1read_miss = " << L1->read_miss << "\tL2read = " << L2->read << "\tL2read_miss = " << L2->read_miss << endl;
                    L1->set_dirty_bit(address);
                }
                else{               // Read Instruction
                    L1->read += 1;
                    // cout << "Read Mode for hit in L1. New L1read_count = " << L1->read << "\tL1read_miss = " << L1->read_miss << "\tL2read = " << L2->read << "\tL2read_miss = " << L2->read_miss << endl;
                }
            }
            else{               // Process when miss in L1
                // cout << "Miss in L1\n";
                total_access_time += L2->memory_access_time;
                // cout << "Checks block existence in L2 \n";
                hit = L2->check_block(address);
                if (hit){     // Process when hits in L2
                    // cout << "Hit in L2\n";
                    if (mode){          // Write Instruction
                        L1->write += 1;
                        L1->write_miss += 1;
                        L2->read += 1;
                        // cout << "Write Mode for hit in L2. New L1read_count = " << L1->read << "\tL1read_miss = " << L1->read_miss << "\tL2read = " << L2->read << "\tL2read_miss = " << L2->read_miss << endl;
                        // L2->set_dirty_bit(address);
                    }
                    else{               // Read Instruction
                        L1->read += 1;
                        L1->read_miss += 1;
                        L2->read += 1;
                        // cout << "Read Mode for hit in L2. New L1read_count = " << L1->read << "\tL1read_miss = " << L1->read_miss << "\tL2read = " << L2->read << "\tL2read_miss = " << L2->read_miss << endl;
                    }
                }
                else{                   // Process when miss in L2
                    // cout << "Miss in L2\n";
                    total_access_time += dram_access_time;
                    if (mode){              // Write Instruction
                        L1->write += 1;
                        L1->write_miss += 1;
                        L2->read += 1;
                        L2->read_miss += 1;
                        // cout << "Write Mode for miss in L2. New L1read_count = " << L1->read << "\tL1read_miss = " << L1->read_miss << "\tL2read = " << L2->read << "\tL2read_miss = " << L2->read_miss << endl;
                    }
                    else{                   // Read Instruction
                        L1->read += 1;
                        L1->read_miss += 1;
                        L2->read += 1;
                        L2->read_miss += 1;
                        // cout << "Read Mode for miss in L2. New L1read_count = " << L1->read << "\tL1read_miss = " << L1->read_miss << "\tL2read = " << L2->read << "\tL2read_miss = " << L2->read_miss << endl;
                    }
                }
                // cout << "Checking L1 writeback or not\n";
                if (L1->check_writeback(address)) {
                    L2->set_dirty_bit(L1->check_writeback(address)-1);
                    L2->update_lru(L1->check_writeback(address)-1);
                    total_access_time += L2->memory_access_time;
                    L2->write += 1;
                    // cout << "Setting dirty bit in L2" << endl;
                }
                if (!hit){
                    // cout << "Inserting block in L2\n";
                    L2->insert_block(0, address);
                    // cout << "Checking L2 writeback or not\n";
                    if (L2->check_writeback(address)){
                        total_access_time += dram_access_time;
                    }
                }
                L2->update_lru(address);
                // cout << "Inserting block in L1\n";
                L1->insert_block(mode, address);
            }
            L1->update_lru(address);
            instruction_count ++;
            // cout << "Time spent = " << total_access_time << endl;
            // cout << endl;
        }
        total_access_time = (L1->read + L1->write)*(L1->memory_access_time) + (L2->read + L2->write)*(L2->memory_access_time) + (L2->read_miss + L2->write_miss + L2->write_back_count)*dram_access_time;
        cout << "Time spent = " << total_access_time << endl;
        L1->print_cache("L1");
        L2->print_cache("L2");
    }
};

#endif