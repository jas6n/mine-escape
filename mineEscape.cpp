// Project Identifier: 19034C8F3B1196BF8E0C6E1C0F973D2FD550B88F

#include <iostream>
#include <iomanip>
#include "xcode_redirect.hpp"
#include <getopt.h>
#include <vector>
#include <queue>
#include <algorithm>
#include "P2random.h"
using namespace std;


static struct option long_options[] = {
    {"help", no_argument, nullptr, 'h' },
    {"stats", required_argument, nullptr, 's'},
    {"median", no_argument, nullptr, 'm'},
    {"verbose", no_argument, nullptr, 'v'},
    { nullptr, 0, nullptr, '\0'}
};

struct Tile {
    int amount = 0;
    bool discovered = false;
    bool tnt_discovered = false;

};

struct Priority_tile {
    size_t row = 0;
    size_t column = 0;
    int amount = 0;
};

bool compare_tiles(const Priority_tile &t1, const Priority_tile &t2){
   


    if (t1.amount == -1 && t2.amount != -1){
        return false;
    } else if (t1.amount != -1 && t2.amount == -1){
        return true;
    } else if (t1.amount != t2.amount){
        return t1.amount > t2.amount;
    } else if (t1.column != t2.column){
        return t1.column > t2.column;
    } else {
        return t1.row > t2.row;
    }


}

struct TileIndex {
    size_t row = 0;
    size_t column = 0;
};


// struct Compared_Tile {

// };

struct Priority_compare {
// checks if priority of t1 < priority of t2
bool operator()(const Priority_tile &t1, const Priority_tile &t2) const{



    if (t1.amount == -1 && t2.amount != -1){
        return false;
    } else if (t1.amount != -1 && t2.amount == -1){
        return true;
    } else if (t1.amount != t2.amount){
        return t1.amount > t2.amount;
    } else if (t1.column != t2.column){
        return t1.column > t2.column;
    } else {
        return t1.row > t2.row;
    }




}
};

class Mine{
vector<vector<Tile>> map;
priority_queue<Priority_tile, vector<Priority_tile>, Priority_compare> dynamites;
priority_queue<Priority_tile, vector<Priority_tile>, Priority_compare> pq_main;
priority_queue<Priority_tile, vector<Priority_tile>, Priority_compare> pq_tnt;
vector<Priority_tile> tile_order; // order of tiles added to vector
priority_queue<double, vector<double>, greater<double>> min_heap_larger_half; // smaller numbers have highest priority
priority_queue<double> max_heap_smaller_half; // larger numbers have higher priority
size_t map_size;
size_t index_1;
size_t index_2;
size_t stats_arg;
double median_value;
int choice;
int index;
int num_tiles;
int rubble_amount;
bool stats;
bool median;
bool verbose;


public:

Mine() : map_size{0}, index_1{0}, index_2{0}, stats_arg{0}, median_value(0), choice{0}, index{0}, num_tiles{0}, rubble_amount{0}, stats{false}, median{false}, verbose{false} {}

void get_commands(int argc, char * argv[]){
    while((choice = getopt_long(argc, argv, "hs:mv", long_options, &index)) != -1){
        switch(choice){
            case 'h':
                cout << "This program escapes a mine or something idk " << '\n';
                exit(0);
                break;

            case 's': {
                stats = true;
                stats_arg = static_cast<size_t>(atoi(optarg));
                break;
            }

            case 'm':
                median = true;
                break;
            
            case 'v':
                verbose = true;
                break;
            
        }  
    }
}

void read_file(){

    char mode;
    string hold;
    int hold_int;

    cin >> mode;

    if (mode != 'R' && mode != 'M'){
        cerr << "First line in file needs to be either M or R idiot" << '\n';
        exit(1);
    }

    cin >> hold;
    cin >> map_size;
    cin >> hold;
    cin >> hold_int;

    if ((hold_int < 0) || (hold_int >= static_cast<int>(map_size))){
        cerr << "Not a proper index dumbass" << '\n';
        exit(1);
    }

    index_1 = static_cast<size_t>(hold_int);


    cin >> hold_int;

    if ((hold_int < 0) || (hold_int >= static_cast<int>(map_size))){
        cerr << "Not a proper index dumbass" << '\n';
        exit(1);
    }
    index_2 = static_cast<size_t>(hold_int);

    stringstream ss;
    if (mode == 'R') {
        // TODO: Read some variables from cin
        uint32_t seed;
        uint32_t max_rubble;
        uint32_t tnt_amt;
        cin >> hold;
        cin >> seed;
        cin >> hold;
        cin >> max_rubble;
        cin >> hold;
        cin >> tnt_amt;
        P2random::PR_init(ss, static_cast<uint32_t>(map_size), seed, max_rubble, tnt_amt);
    }  // if ..'R'

    // If map mode is on, get input from cin,
    // otherwise get input from the stringstream
    istream &inputStream = (mode == 'M') ? cin : ss;

    map.resize(map_size, vector<Tile>(map_size));

    for (size_t i = 0; i < map_size; ++i){
        for (size_t j = 0; j < map_size; ++j){
            inputStream >> hold_int;
            map[i][j].amount = hold_int;
        }
    }
}

// void prepare_stats(const int amt){
//     if (amt == 1){

//     }
// }

void get_median(const double amt){

    if (min_heap_larger_half.size() > max_heap_smaller_half.size()){
        if (amt > median_value){
            double hold = min_heap_larger_half.top();
            min_heap_larger_half.pop();
            max_heap_smaller_half.push(hold);
            min_heap_larger_half.push(amt);
        } else {
            max_heap_smaller_half.push(amt);
        }

        median_value = (max_heap_smaller_half.top() + min_heap_larger_half.top()) / 2;

    } else if (min_heap_larger_half.size() < max_heap_smaller_half.size()) {
        if (amt < median_value){
            double hold = max_heap_smaller_half.top();
            max_heap_smaller_half.pop();
            min_heap_larger_half.push(hold);
            max_heap_smaller_half.push(amt);
        } else {
            min_heap_larger_half.push(amt);
        }
        median_value = (max_heap_smaller_half.top() + min_heap_larger_half.top()) / 2;
    } else {
        if (amt < median_value){
            max_heap_smaller_half.push(amt);
            median_value = max_heap_smaller_half.top();
        } else {
            min_heap_larger_half.push(amt);
            median_value = min_heap_larger_half.top();
        }

    }

    
    cout << "Median difficulty of clearing rubble is: " << median_value << '\n';

}


void add_to_discovered(){


    if (index_1 != 0){
        if (!map[index_1-1][index_2].discovered){
            map[index_1-1][index_2].discovered = true;
            Priority_tile t;
            t.amount = map[index_1-1][index_2].amount;
            t.row = index_1 - 1;
            t.column = index_2;
            pq_main.push(t);
        }
    }


    if (index_1 != map_size - 1){
        if (!map[index_1+1][index_2].discovered){
            map[index_1+1][index_2].discovered = true;
            Priority_tile t;
            t.amount = map[index_1+1][index_2].amount;
            t.row = index_1 + 1;
            t.column = index_2;
            pq_main.push(t);
        }
    }

    if (index_2 != 0){
        if (!map[index_1][index_2-1].discovered){
            map[index_1][index_2-1].discovered = true;
            Priority_tile t;
            t.amount = map[index_1][index_2-1].amount;
            t.row = index_1;
            t.column = index_2 - 1;
            pq_main.push(t);
        }
    }

    if (index_2 != map_size - 1){
        if (!map[index_1][index_2+1].discovered){
            map[index_1][index_2+1].discovered = true;
            Priority_tile t;
            t.amount = map[index_1][index_2 + 1].amount;
            t.row = index_1;
            t.column = index_2 + 1;
            pq_main.push(t);
        }
    }

}

void tnt_add_to_discovered(){


    if (index_1 != 0){
        if ((!map[index_1-1][index_2].discovered) && (map[index_1-1][index_2].amount != -1)){
            map[index_1-1][index_2].discovered = true;
            Priority_tile t;
            t.amount = map[index_1-1][index_2].amount;
            t.row = index_1 - 1;
            t.column = index_2;
            pq_main.push(t);
        }
    }


    if (index_1 != map_size - 1){
        if ((!map[index_1+1][index_2].discovered) && (map[index_1+1][index_2].amount != -1)){
            map[index_1+1][index_2].discovered = true;
            Priority_tile t;
            t.amount = map[index_1+1][index_2].amount;
            t.row = index_1 + 1;
            t.column = index_2;
            pq_main.push(t);
        }
    }

    if (index_2 != 0){
        if ((!map[index_1][index_2-1].discovered) && (map[index_1][index_2-1].amount != -1)){
            map[index_1][index_2-1].discovered = true;
            Priority_tile t;
            t.amount = map[index_1][index_2-1].amount;
            t.row = index_1;
            t.column = index_2 - 1;
            pq_main.push(t);
        }
    }

    if (index_2 != map_size - 1){
        if ((!map[index_1][index_2+1].discovered) && (map[index_1][index_2+1].amount != -1)){
            map[index_1][index_2+1].discovered = true;
            Priority_tile t;
            t.amount = map[index_1][index_2+1].amount;
            t.row = index_1;
            t.column = index_2 + 1;
            pq_main.push(t);
        }
    }

}

bool free(){
    if (index_1 == 0 || index_1 == map_size - 1 || index_2 == 0 || index_2 == map_size - 1) {
        return true;
    }
    return false;
}


// activates after tnt explodes
void tnt_steps(size_t ind1, size_t ind2){

    // shouldnt add investigated to priority queue
    if ((!map[ind1][ind2].tnt_discovered) && (map[ind1][ind2].amount != 0)){

        // add it to tnt priority queue
        Priority_tile t;
        t.amount = map[ind1][ind2].amount;
        t.row = ind1;
        t.column = ind2;
        pq_tnt.push(t);
        map[ind1][ind2].tnt_discovered = true;
        if (map[ind1][ind2].amount == -1){
            Priority_tile t;
            t.amount = map[ind1][ind2].amount;
            t.row = ind1;
            t.column = ind2;
            dynamites.push(t);
        }

    }
}

void activate_tnt(size_t i1, size_t i2){

    // adds tiles adjacent to original tnt to pq main
    if (verbose){
        cout << "TNT explosion at [" << i1 << ","<< i2 << "]!"<< '\n';
    }
    if (stats){
        Priority_tile t;
        t.amount = map[i1][i2].amount;
        t.row = i1;
        t.column = i2;
        tile_order.push_back(t);
    }
    tnt_add_to_discovered();
    map[i1][i2].amount = 0;

    map[i1][i2].tnt_discovered = true;

    bool still_tnt = true;


    // makes priority queue without -1s
    while (still_tnt){
        // creates priority queue
        // if (map[i1][i2].amount == -1){

        if (i1 > 0){
            tnt_steps(i1-1, i2);
        }

        if (i1 < map_size - 1){
            tnt_steps(i1+1, i2); 
        }

        if (i2 > 0){
            tnt_steps(i1, i2-1); 
        }

        if (i2 < map_size - 1){
            tnt_steps(i1, i2+1); 
        }
            
        // }

        // determines when to end priority queue
        if (!dynamites.empty()){
            Priority_tile t = dynamites.top();
            if (verbose){
                cout << "TNT explosion at [" << t.row << ","<< t.column << "]!"<< '\n';
            }
            if (stats){
                tile_order.push_back(t);
            }
            // if (!map[i1][i2].discovered){
            //     map[i1][i2].discovered = true;
            //     pq_main.push(map[i1][i2]);
            // }
            i1 = t.row;
            i2 = t.column;
            dynamites.pop();
        } else {
            still_tnt = false;
        }


    }   



    while (!pq_tnt.empty()){
        Priority_tile t = pq_tnt.top();

        map[t.row][t.column].amount = 0;

        
        map[t.row][t.column].discovered = true; // might be a problem, gotta set t.amount = 0

            if (t.amount == -1){
                
            } else {
                if (verbose){
                    cout << "Cleared by TNT: " << t.amount << " at [" << t.row << "," << t.column << "]" << '\n';
                }
                if (median){
                    get_median(static_cast<double>(t.amount));
                    
                }
                if (stats){
                    tile_order.push_back(t);
                    // prepare_stats(t.amount);
                }
                num_tiles += 1;
                rubble_amount += t.amount;
            }
        t.amount = 0;    
        pq_main.push(t);    
        pq_tnt.pop();
    }

}   


void escape(){

    map[index_1][index_2].discovered = true;


    Priority_tile t;
    t.amount = map[index_1][index_2].amount;
    t.row = index_1;
    t.column = index_2;
    pq_main.push(t);

    // start the loop
    // add to queue if not discovered
    bool cont = true;

    while (cont){
        // investigate stage
        Priority_tile tt = pq_main.top();

        index_1 = tt.row;
        index_2 = tt.column;

        pq_main.pop();
        // blow up tnt
        if (map[index_1][index_2].amount == -1){
            activate_tnt(index_1, index_2);
        }

        // clear the rubble
        if (map[index_1][index_2].amount != 0){
            if (verbose){
                cout << "Cleared: " <<  map[index_1][index_2].amount << " at [" << index_1 << "," << index_2 << "]" << '\n';
            } 
            if (median){
                get_median(static_cast<double>(map[index_1][index_2].amount));

            }
            if (stats){
                // prepare_stats(map[index_1][index_2].amount);
                Priority_tile t1;
                t1.amount = map[index_1][index_2].amount;
                t1.row = index_1;
                t1.column = index_2;
                tile_order.push_back(t1);

            }
            rubble_amount += map[index_1][index_2].amount; 
            map[index_1][index_2].amount = 0;// add to rubble amount
            num_tiles += 1; // add to tiles cleared
        }
        // make sure you add first and last tiles

        // check if escaped

        // discover if still there

       
         if (free()){
            cont = false;
            break;
        }

        add_to_discovered();
        // 
        
    }
}

void summary(){
    cout << "Cleared " << num_tiles << " tiles containing " << rubble_amount << " rubble and escaped." << '\n';
    if (stats){
        if (stats_arg > tile_order.size()){
            stats_arg = tile_order.size();
        }
        // show order
        cout << "First tiles cleared:" << '\n';
        for (size_t i = 0; i < stats_arg; ++i){
            if (tile_order[i].amount != -1){
                cout << tile_order[i].amount << " at [" << tile_order[i].row << ","<<tile_order[i].column << "]" << '\n';
            } else {
                cout << "TNT at [" << tile_order[i].row << "," << tile_order[i].column << "]" << '\n';
            }
        }
        cout << "Last tiles cleared:" << '\n';
        for (size_t i = 0; i < stats_arg; ++i){
            if (tile_order[tile_order.size()-i-1].amount != -1){
                cout << tile_order[tile_order.size()-i-1].amount << " at [" << tile_order[tile_order.size()-i-1].row << ","<<tile_order[tile_order.size()-i-1].column << "]" << '\n';
            } else {
                cout << "TNT at [" << tile_order[tile_order.size()-i-1].row << "," << tile_order[tile_order.size()-i-1].column << "]" << '\n';
            }
        }

        // sort the vector
        sort(tile_order.begin(), tile_order.end(), compare_tiles);

        cout << "Easiest tiles cleared:" << '\n';
        for (size_t i = 0; i < stats_arg; ++i){
            if (tile_order[tile_order.size()-i-1].amount != -1){
                cout << tile_order[tile_order.size()-i-1].amount << " at [" << tile_order[tile_order.size()-i-1].row << ","<<tile_order[tile_order.size()-i-1].column << "]" << '\n';
            } else {
                cout << "TNT at [" << tile_order[tile_order.size()-i-1].row << "," << tile_order[tile_order.size()-i-1].column << "]" << '\n';
            }
        }

        cout << "Hardest tiles cleared:" << '\n';
        for (size_t i = 0; i < stats_arg; ++i){
            if (tile_order[i].amount != -1){
                cout << tile_order[i].amount << " at [" << tile_order[i].row << ","<<tile_order[i].column << "]" << '\n';
            } else {
                cout << "TNT at [" << tile_order[i].row << "," << tile_order[i].column << "]" << '\n';
            }
        }


    }
}


};



int main(int argc, char* argv[]){
    ios_base::sync_with_stdio(false);
    xcode_redirect(argc, argv);
    cout << fixed << setprecision(2);

    Mine mine;
    mine.get_commands(argc, argv);
    mine.read_file();

    mine.escape();

    mine.summary();

    return 0;
}

// pass by reference and make sure it works by running it against the zero test
