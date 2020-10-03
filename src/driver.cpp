#include "cache_hierarchy.h"
#include <pthread.h>
#include "knobs.h"

using namespace std;

int numtraces = 0;
string trace_name = "";
Cache_Hierarchy *Simulation[3] = {NULL, NULL, NULL};

int num_sims = 3;
bool sims[3] = {true, true, true};
bool mt_enabled = true;

void *Process(void *ptr);
void Parse (int argc, char *argv[]);
void Usage();
void MT();

int main(int argc, char *argv[]){

    Parse(argc, argv);

    if (mt_enabled && num_sims > 1){
        MT();
    }
    else {
        for (int p = 0; p < 3; p++){
            if (sims[p]){
                Process(Simulation[p]);
            }
        }
    }

    for (int p = 0; p < 3; p++){
        if (sims[p]){
            Simulation[p]->Print_Stats();
        }
    }

    return 0;
}

void *Process(void *ptr){
    Cache_Hierarchy *p = (Cache_Hierarchy *)ptr;
    char i_or_d;	
    char type;	
    uint64_t addr;	
    uint32_t pc;
    for (int k = 0; k < numtraces; k++) {
        string input_file = "./traces/" + trace_name + to_string(k);
        FILE *fp = fopen(input_file.c_str(), "rb");
        assert(fp != NULL);
        while (!feof(fp)) {
            fread(&i_or_d, sizeof(char), 1, fp);
            fread(&type, sizeof(char), 1, fp);
            fread(&addr, sizeof(unsigned long long), 1, fp);
            fread(&pc, sizeof(unsigned), 1, fp);
            
            // cout << i_or_d << " " << type << " " << addr << " " << pc << endl;
            if (type){
                // cout << addr << endl;
                p->Simulate(addr);
            }
        }
        fclose(fp);
    }
}

void Parse (int argc, char *argv[]){
    if (argc < 5){
        Usage();
        exit(0);
    }
    for (int i = 1; i < argc; i += 2){
        if (strcmp(argv[i], "-simulate") == 0){
            if (strcmp(argv[i+1], "all") == 0){
                num_sims = 3;
            }
            else{
                sims[0] = sims[1] = sims[2] = false;
                num_sims = 0;
                string sim_types(argv[i+1]);
                if (sim_types.find("incl") != string::npos){
                    sims[0] = true;
                    num_sims++;
                }
                if (sim_types.find("excl") != string::npos){
                    sims[1] = true;
                    num_sims++;
                }
                if (sim_types.find("nine") != string::npos){
                    sims[2] = true;
                    num_sims++;
                }
            }
        }
        else if (strcmp(argv[i], "-parts") == 0){
            numtraces = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "-trace") == 0){
            trace_name = string(argv[i+1]);
        }
        else if (strcmp(argv[i], "-mt") == 0){
            if (strcmp(argv[i+1], "disable") == 0){
                mt_enabled = false;
            }
        }
        else {
            cout << "Invalid flag" << endl;
            Usage();
            exit(0);
        }
    }
    assert(numtraces > 0);
    assert(trace_name != "");
    assert(num_sims > 0);
    
    cout << "Trace: " << trace_name << " parts: " << numtraces << endl;
    trace_name.append("_");
    if (sims[0]){
        cout << left << setw(30) << "Inclusive policy: " << "enabled" << endl;
        Simulation[0] = new Cache_Hierarchy(INCLUSIVE);
    }
    if (sims[1]){
        cout << left << setw(30) << "Exclusive policy: " << "enabled" << endl;
        Simulation[1] = new Cache_Hierarchy(EXCLUSIVE);
    }
    if (sims[2]){
        cout << left << setw(30) << "NINE policy: " << "enabled" << endl;
        Simulation[2] = new Cache_Hierarchy(NINE);
    }
}

void MT(){
    pthread_t threads[num_sims];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int t_num = 0;
    void *status;
    for (int p = 0; p < 3; p++){
        if (sims[p]){
            pthread_create(&threads[t_num], &attr, Process, (void *)Simulation[p]);
            t_num++;
        }
    }
    pthread_attr_destroy(&attr);
    for (int t = 0; t < t_num; t++){
        pthread_join(threads[t], &status);
    }
}

void Usage(){
    cout << "Usage:     ./driver -trace <trace_name> -parts <number_of_parts> ..." << endl <<
            "Example:   ./driver -trace abc -parts 5 -mt enable -simulate incl,excl" << endl <<
            "-trace     Specify name of trace [REQUIRED]" << endl <<
            "-parts     Specify number of trace parts, minimum value = 1 [REQUIRED]" << endl <<
            "-mt        enable/ disable multi-threading [OPTIONAL, default = enable]" << endl <<
            "-simulate  Simulate all/ incl/ excl/ nine policies [OPTIONAL, default = all]" << endl;
}
