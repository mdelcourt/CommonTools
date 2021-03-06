// Read some trees, output an histos
// A. Mertens (September 2015)

#include <json/json.h>

#include <iostream>
#include <fstream>


#include <TVirtualTreePlayer.h>
#include <TMultiDrawTreePlayer.h>
#include <TChain.h>
#include <TTree.h>
#include <TFile.h>
#include <TDirectory.h>


int main( int argc, char* argv[]){

    if (argc != 3) {
        std::cerr << "use as " << argv[0] << " plots.json files.json\n";
        return 0;
    }

    std::cout << "samples : " << argv[1] << " plots : " << argv[2] << std::endl;

    const char* plots_json = argv[1];
    const char* sample_json = argv[2];

    // Setting the TVirtualTreePlayer
    TVirtualTreePlayer::SetPlayer("TMultiDrawTreePlayer");

    // Getting the list of samples    
    Json::Value samplesroot;
    std::ifstream config_doc(sample_json, std::ifstream::binary);

    Json::Reader samplesreader;
    bool parsingSuccessful = samplesreader.parse(config_doc, samplesroot, false);
    if (!parsingSuccessful) { return 0;}
    // Let's extract the array contained in the root object
    Json::Value::Members samples_str = samplesroot.getMemberNames();

    // Looping over the different samples
    for (unsigned int index = 0; index < samplesroot.size(); index++){    

        const Json::Value samplearray = samplesroot[samples_str.at(index)];

        std::string tree_name = samplearray.get("tree_name","ASCII").asString();
        std::string path = samplearray.get("path","ASCII").asString();
        std::string db_name = samplearray.get("db_name","ASCII").asString();
        std::string sample_cut = samplearray.get("sample_cut","ASCII").asString();

        std::cout << "running on sample : " << samples_str.at(index) << std::endl;

        TChain* t = new TChain(tree_name.c_str());

        std::string infiles = path+"/*.root";

        t->Add(infiles.c_str());

        TFile* outfile = new TFile((db_name+"_histos.root").c_str(),"recreate");

        TMultiDrawTreePlayer* p = dynamic_cast<TMultiDrawTreePlayer*>(t->GetPlayer());

        // extracting the plots to draw
        Json::Value plotsroot;
        std::ifstream config_doc(plots_json, std::ifstream::binary);
        Json::Reader plotsreader;
        bool parsingSuccessful = plotsreader.parse(config_doc, plotsroot, false);
        if (!parsingSuccessful) { return 0;}
        // Let's extract the array contained in the root object
        Json::Value::Members plots_str = plotsroot.getMemberNames();

        // Looping over the different plots
        std::cout << "    plotting : ";

        for (unsigned int index_plot = 0; index_plot < plotsroot.size(); index_plot++){

             const Json::Value array = plotsroot[plots_str.at(index_plot)];
             std::string variable = array.get("variable","ASCII").asString();
             std::string plotCuts = array.get("plot_cut","ASCII").asString();
             std::string binning = array.get("binning","ASCII").asString();

             std::cout << plots_str.at(index_plot) << " , " ;
             std::string plot_var = variable+">>"+plots_str.at(index_plot)+binning;
             p->queueDraw(plot_var.c_str(), plotCuts.c_str());

        }
        std::cout << std::endl;

        p->execute();
        for (unsigned int index_plot = 0; index_plot < plotsroot.size(); index_plot++){
            gDirectory->Get(plots_str.at(index_plot).c_str())->Write();
        }

        outfile->Write();
        delete outfile;


    }

}

