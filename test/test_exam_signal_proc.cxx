#include "WireCellSigProc/OmnibusSigProc.h"
#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Waveform.h"



#include "WireCellIface/SimpleFrame.h"
#include "WireCellIface/SimpleTrace.h"

#include "WireCellUtil/ExecMon.h"

#include <iostream>
#include <string>
#include <numeric>		// iota

#include "TH2F.h"
#include "TFile.h"
#include "TTree.h"


using namespace WireCell;
using namespace std;

const string url_test = "/data0/bviren/data/uboone/test_3455_0.root"; // big!


void save_into_file(const char* filename,IFrame::pointer frame_decon, int nrebin){
  TFile *file1 = new TFile(filename);

  TFile *file = new TFile("temp.root","RECREATE");
  TTree *Trun = ((TTree*)file1->Get("Trun"))->CloneTree();
  Trun->SetDirectory(file);

  TH2I *hu_orig = (TH2I*)file1->Get("hu_orig");
  TH2I *hv_orig = (TH2I*)file1->Get("hv_orig");
  TH2I *hw_orig = (TH2I*)file1->Get("hw_orig");

  hu_orig->SetDirectory(file);
  hv_orig->SetDirectory(file);
  hw_orig->SetDirectory(file);
  
  int nwire_u = hu_orig->GetNbinsX();
  int nwire_v = hv_orig->GetNbinsX();
  int nwire_w = hw_orig->GetNbinsX();
  int nticks = hu_orig->GetNbinsY();

  
  TH2F *hu_raw = (TH2F*)file1->Get("hu_raw");
  TH2F *hv_raw = (TH2F*)file1->Get("hv_raw");
  TH2F *hw_raw = (TH2F*)file1->Get("hw_raw");

  hu_raw->SetDirectory(file);
  hv_raw->SetDirectory(file);
  hw_raw->SetDirectory(file);

  
  TH1F *hu_baseline = (TH1F*)file1->Get("hu_baseline");
  TH1F *hv_baseline = (TH1F*)file1->Get("hv_baseline");
  TH1F *hw_baseline = (TH1F*)file1->Get("hw_baseline");
  
  hu_baseline->SetDirectory(file);
  hv_baseline->SetDirectory(file);
  hw_baseline->SetDirectory(file);

  
  // temporary ...  need a data structure to load the threshold ... 
  TH1F *hu_threshold = (TH1F*)file1->Get("hu_threshold");
  TH1F *hv_threshold = (TH1F*)file1->Get("hv_threshold");
  TH1F *hw_threshold = (TH1F*)file1->Get("hw_threshold");
  
  hu_threshold->SetDirectory(file);
  hv_threshold->SetDirectory(file);
  hw_threshold->SetDirectory(file);
  


  
  // temporary ...

  
  TH2F *hu_decon = new TH2F("hu_decon","hu_decon",nwire_u,-0.5,nwire_u-0.5,int(nticks/nrebin),0,nticks);
  TH2F *hv_decon = new TH2F("hv_decon","hv_decon",nwire_v,-0.5+nwire_u,nwire_v-0.5+nwire_u,int(nticks/nrebin),0,nticks);
  TH2F *hw_decon = new TH2F("hw_decon","hw_decon",nwire_w,-0.5+nwire_u+nwire_v,nwire_w-0.5+nwire_u+nwire_v,int(nticks/nrebin),0,nticks);

  
  auto traces = frame_decon->traces();
  for (auto trace : *traces.get()) {
    int tbin = trace->tbin();
    int ch = trace->channel();
    auto charges = trace->charge();
    if (ch < nwire_u){
      int counter = 0;
      int rebin_counter = 0;
      float acc_charge = 0;
      
      for (auto q : charges) {
	if (rebin_counter < nrebin){
	  acc_charge += q;
	  rebin_counter ++;
	}
	if (rebin_counter == nrebin){
	  counter ++;
	  hu_decon->SetBinContent(ch+1,tbin+counter,acc_charge); 
	  //reset ... 
	  rebin_counter = 0;
	  acc_charge = 0;
	}
      }
    }else if (ch < nwire_v + nwire_u){

      int counter = 0;
      int rebin_counter = 0;
      float acc_charge = 0;
      
      for (auto q : charges) {
	
	if (rebin_counter < nrebin){
	  acc_charge += q;
	  rebin_counter ++;
	}
	if (rebin_counter == nrebin){
	  counter ++;
	  hv_decon->SetBinContent(ch+1-nwire_u,tbin+counter,acc_charge); 
	  //reset ... 
	  rebin_counter = 0;
	  acc_charge = 0;
	}
      }

      
      // int counter = 0;
      // for (auto q : charges) {
      // 	counter ++;
      // 	hv_decon->SetBinContent(ch+1-nwire_u,tbin+counter,q); 
	
      // }
    }else{

      int counter = 0;
      int rebin_counter = 0;
      float acc_charge = 0;
      
      for (auto q : charges) {
	
	if (rebin_counter < nrebin){
	  acc_charge += q;
	  rebin_counter ++;
	}
	if (rebin_counter == nrebin){
	  counter ++;
	  hw_decon->SetBinContent(ch+1-nwire_u-nwire_v,tbin+counter,acc_charge); 
	  //reset ... 
	  rebin_counter = 0;
	  acc_charge = 0;
	}
      }
      
      // int counter = 0;
      // for (auto q : charges) {
      // 	counter ++;
      // 	hw_decon->SetBinContent(ch+1-nwire_u-nwire_v,tbin+counter,q); 
	
      // }
    }
  }


  
   // save bad channels 
  TTree *T_bad = new TTree("T_bad","T_bad");
  int chid, plane, start_time,end_time;
  T_bad->Branch("chid",&chid,"chid/I");
  T_bad->Branch("plane",&plane,"plane/I");
  T_bad->Branch("start_time",&start_time,"start_time/I");
  T_bad->Branch("end_time",&end_time,"end_time/I");
  T_bad->SetDirectory(file);

  TTree *T_lf = new TTree("T_lf","T_lf");
  int channel;
  T_lf->Branch("channel",&channel,"channel/I");
  

  Waveform::ChannelMaskMap input_cmm = frame_decon->masks();
  for (auto const& it: input_cmm) {

    if (it.first == "bad"){ // save bad ... 
      //std::cout << "Xin1: " << it.first << " " << it.second.size() << std::endl;
      for (auto const &it1 : it.second){
	chid = it1.first;
	if (chid < nwire_u){
	  plane = 0;
	}else if (chid < nwire_v + nwire_u){
	  plane = 1;
	}else{
	  plane = 2;
	}
	//std::cout << "Xin1: " << chid << " " << plane << " " << it1.second.size() << std::endl;
	for (size_t ind = 0; ind < it1.second.size(); ++ind){
	  start_time = it1.second[ind].first;
	  end_time = it1.second[ind].second;
	  T_bad->Fill();
	}
      }
    }else if (it.first =="lf_noisy"){
      for (auto const &it1 : it.second){
	channel = it1.first;
	T_lf->Fill();
      }
      
    }else if (it.first=="threshold"){
       for (auto const &it1 : it.second){
	 chid = it1.first;
	 float threshold = it1.second[0].first/it1.second[0].second;
	 if (chid < nwire_u){
	   hu_threshold->SetBinContent(chid+1,threshold*nrebin*3.0);
	 }else if (chid < nwire_u+nwire_v){
	   hv_threshold->SetBinContent(chid+1-nwire_u,threshold*nrebin*3.0);
	 }else{
	   hw_threshold->SetBinContent(chid+1-nwire_u-nwire_v,threshold*nrebin*3.0);
	 }
	 
       }
    }

    
  }

  file->Write();
  file->Close();
  
}


class XinFileIterator {
  TH2* hist[3];		// per plane
  WireCell::Waveform::ChannelMaskMap ret;
  TFile *file;
public:
    XinFileIterator(const char* filename, const char* histtype="raw") {
      file = TFile::Open(filename);
      string uvw = "uvw";
      for (int ind=0; ind<3; ++ind) {
	auto c = uvw[ind];
	std::string name = Form("h%c_%s", c, histtype);
	cerr << "Loading " << name << endl;
	hist[ind] = (TH2*)file->Get(name.c_str());
      }
      
      TTree *T_bad = (TTree*)file->Get("T_bad");
      int chid, plane, start_time,end_time;
      T_bad->SetBranchAddress("chid",&chid);
      T_bad->SetBranchAddress("plane",&plane);
      T_bad->SetBranchAddress("start_time",&start_time);
      T_bad->SetBranchAddress("end_time",&end_time);
      
      for (int i=0;i!=T_bad->GetEntries();i++){
	T_bad->GetEntry(i);
	WireCell::Waveform::BinRange chirped_bins;
	chirped_bins.first = start_time;
	chirped_bins.second = end_time;
	ret["bad"][chid].push_back(chirped_bins);
      }
      
      
      TTree *T_lf = (TTree*)file->Get("T_lf");
      int channel;
      T_lf->SetBranchAddress("channel",&channel);
      for (int i=0;i!=T_lf->GetEntries();i++){
	T_lf->GetEntry(i);
	WireCell::Waveform::BinRange chirped_bins;
	chirped_bins.first = 0;
	chirped_bins.second = hist[0]->GetNbinsY();
	ret["lf_noisy"][channel].push_back(chirped_bins);
      }
      delete T_lf;
      delete T_bad;
      
	//file->Close();
	//delete file;
    }

    int plane(int ch) {
	if (ch < 2400) return 0;
	if (ch < 2400+2400) return 1;
	return 2;
    }
    int index(int ch) {
	if (ch < 2400) return ch;
	if (ch < 2400+2400) return ch-2400;
	return ch-2400-2400;
    }

    vector<float> at(int ch) {
	TH2* h = hist[plane(ch)];
	int ind = index(ch);
	vector<float> ret(9600);
	for (int itick=0; itick<9600; ++itick) {
	    ret[itick] = h->GetBinContent(ind+1, itick+1);
	}
	return ret;
    }

  void clear(){
    delete hist[0];
    delete hist[1];
    delete hist[2];
    
    file->Close();
    delete file;
  }
  
    /// Return a frame, the one and only in the file.
    IFrame::pointer frame() {
	ITrace::vector traces;

	int chindex=0;
	for (int iplane=0; iplane<3; ++iplane) {
	    TH2* h = hist[iplane];

	    int nchannels = h->GetNbinsX();
	    int nticks = h->GetNbinsY();

	    cerr << "plane " << iplane << ": " << nchannels << " X " << nticks << endl;

	    double qtot = 0.0;
	    for (int ich = 1; ich <= nchannels; ++ich) {
		ITrace::ChargeSequence charges;
		for (int itick = 1; itick <= nticks; ++itick) {
		    auto q = h->GetBinContent(ich, itick);
		    charges.push_back(q);
		    qtot += q;
		}
		SimpleTrace* st = new SimpleTrace(chindex, 0.0, charges);
		traces.push_back(ITrace::pointer(st));
		++chindex;
		//cerr << "qtot in plane/ch/index "
		//     << iplane << "/" << ich << "/" << chindex << " = " << qtot << endl;
	    }
	}
	SimpleFrame* sf = new SimpleFrame(0, 0, traces, 0.5*units::microsecond, ret);
	return IFrame::pointer(sf);
    }
    
};




int main(int argc, char* argv[])
{
    if (argc < 2) {
	cerr << "This test needs an input data file.  Legend has it that one is found at " << url_test << endl;
	return 1;
    }

    PluginManager& pm = PluginManager::instance();
    pm.add("WireCellGen");
    pm.add("WireCellSigProc");

    string filenames[4] = {
      "microboone-noise-spectra-v2.json.bz2",
      "garfield-1d-3planes-21wires-6impacts-v6.json.bz2",
      "microboone-celltree-wires-v2.json.bz2",
      "ub-10-wnormed.json.bz2",
    };
    
    // do the geometry ... 
    {
      auto anodecfg = Factory::lookup<IConfigurable>("AnodePlane");
      auto cfg = anodecfg->default_configuration();
      cfg["fields"] = filenames[3];
      cfg["wires"] = filenames[2];
      anodecfg->configure(cfg);
    }

    // add the response function ...
    {
      auto ifrcfg = Factory::lookup<IConfigurable>("FieldResponse");
      auto cfg = ifrcfg->default_configuration();
      cfg["filename"] = filenames[3];
      ifrcfg->configure(cfg);
    }

    // add the filters
    {
      // Tight Gaussian filters
      {
	auto incrcfg = Factory::lookup<IConfigurable>("HfFilter","Gaus_tight");
	auto cfg = incrcfg->default_configuration();
	cfg["nbins"] = 9594;
	cfg["max_freq"] = 1 * units::megahertz;
	cfg["sigma"] = 1.11408e-01 * units::megahertz;
	cfg["power"] = 2;
	cfg["flag"] = true;
	incrcfg->configure(cfg);
      }

      // Tight Wiener filters for U for ROI finding
      {
	auto incrcfg = Factory::lookup<IConfigurable>("HfFilter","Wiener_tight_U");
	auto cfg = incrcfg->default_configuration();
	cfg["nbins"] = 9594;
	cfg["max_freq"] = 1 * units::megahertz;
	cfg["sigma"] = 5.75416e+01/800.*2 * units::megahertz;
	cfg["power"] = 4.10358e+00;
	cfg["flag"] = true;
	incrcfg->configure(cfg);
      }
      
      // Tight Wiener filters for V for ROI finding
      {
	auto incrcfg = Factory::lookup<IConfigurable>("HfFilter","Wiener_tight_V");
	auto cfg = incrcfg->default_configuration();
	cfg["nbins"] = 9594;
	cfg["max_freq"] = 1 * units::megahertz;
	cfg["sigma"] = 5.99306e+01/800.*2* units::megahertz;
	cfg["power"] = 4.20820e+00;
	cfg["flag"] = true;
	incrcfg->configure(cfg);
      }
      
      // Tight Wiener filters for W 
      {
	auto incrcfg = Factory::lookup<IConfigurable>("HfFilter","Wiener_tight_W");
	auto cfg = incrcfg->default_configuration();
	cfg["nbins"] = 9594;
	cfg["max_freq"] = 1 * units::megahertz;
	cfg["sigma"] = 5.88802e+01/800.*2  * units::megahertz;
	cfg["power"] = 4.17455e+00;
	cfg["flag"] = true;
	incrcfg->configure(cfg);
      }
      
      
      // Wide Wiener filters for U for hit
      {
	auto incrcfg = Factory::lookup<IConfigurable>("HfFilter","Wiener_wide_U");
	auto cfg = incrcfg->default_configuration();
	cfg["nbins"] = 9594;
	cfg["max_freq"] = 1 * units::megahertz;
	cfg["sigma"] = 1.78695e+01/200.*2.  * units::megahertz;
	cfg["power"] = 5.33129e+00;
	cfg["flag"] = true;
	incrcfg->configure(cfg);
      }

      
      // Wide Wiener filters for V for hit
      {
	auto incrcfg = Factory::lookup<IConfigurable>("HfFilter","Wiener_wide_V");
	auto cfg = incrcfg->default_configuration();
	cfg["nbins"] = 9594;
	cfg["max_freq"] = 1 * units::megahertz;
	cfg["sigma"] = 1.84666e+01/200.*2.  * units::megahertz;
	cfg["power"] = 5.60489e+00;
	cfg["flag"] = true;
	incrcfg->configure(cfg);
      }
      
      // Wide Wiener filters for W for hit
      {
	auto incrcfg = Factory::lookup<IConfigurable>("HfFilter","Wiener_wide_W");
	auto cfg = incrcfg->default_configuration();
	cfg["nbins"] = 9594;
	cfg["max_freq"] = 1 * units::megahertz;
	cfg["sigma"] = 1.83044e+01/200.*2. * units::megahertz;
	cfg["power"] = 5.44945e+00;
	cfg["flag"] = true;
	incrcfg->configure(cfg);
      }
      
      // Wide Gaussian filters for charge
      {
	auto incrcfg = Factory::lookup<IConfigurable>("HfFilter","Gaus_wide");
	auto cfg = incrcfg->default_configuration();
	cfg["nbins"] = 9594;
	cfg["max_freq"] = 1 * units::megahertz;
	cfg["sigma"] = 0.14 * units::megahertz;
	cfg["power"] = 2;
	cfg["flag"] = true;
	incrcfg->configure(cfg);
      }
      
      // Tight low frequency filter for ROI
      {
	auto incrcfg = Factory::lookup<IConfigurable>("LfFilter","ROI_tight_lf");
	auto cfg = incrcfg->default_configuration();
	cfg["nbins"] = 9594;
	cfg["max_freq"] = 1 * units::megahertz;
	cfg["tau"] = 0.02 * units::megahertz;
	incrcfg->configure(cfg);
      }

      {
	auto incrcfg = Factory::lookup<IConfigurable>("LfFilter","ROI_tighter_lf");
	auto cfg = incrcfg->default_configuration();
	cfg["nbins"] = 9594;
	cfg["max_freq"] = 1 * units::megahertz;
	cfg["tau"] = 0.1 * units::megahertz;
	incrcfg->configure(cfg);
      }
      
      // Loose low frequency filter for ROI
      {
	auto incrcfg = Factory::lookup<IConfigurable>("LfFilter","ROI_loose_lf");
	auto cfg = incrcfg->default_configuration();
	cfg["nbins"] = 9594;
	cfg["max_freq"] = 1 * units::megahertz;
	cfg["tau"] = 0.0025 * units::megahertz;
	incrcfg->configure(cfg);
      }
      
      // Wire Filter for induction planes
      {
	auto incrcfg = Factory::lookup<IConfigurable>("HfFilter","Wire_ind");
	auto cfg = incrcfg->default_configuration();
	cfg["nbins"] = 2400;
	cfg["max_freq"] = 1 ;
	cfg["sigma"] = 1./sqrt(3.1415926)*1.4 ;
	cfg["power"] = 2;
	cfg["flag"] = false;
	incrcfg->configure(cfg);
      }
      
      // Wire Filter for collection planes 
      {
	auto incrcfg = Factory::lookup<IConfigurable>("HfFilter","Wire_col");
	auto cfg = incrcfg->default_configuration();
	cfg["nbins"] = 3456;
	cfg["max_freq"] = 1 ;
	cfg["sigma"] = 1.0/sqrt(3.1415926)*3.0 ;
	cfg["power"] = 2;
	cfg["flag"] = false;
	incrcfg->configure(cfg);
      }
    }

    // per channel response 
    {
      const std::string cr_tn = "PerChannelResponse";
      const std::string pcr_filename = "calib_resp_v1.json.bz2";
      auto icrcfg = Factory::lookup<IConfigurable>(cr_tn);
      auto cfg = icrcfg->default_configuration();
      cfg["filename"] = pcr_filename;
      icrcfg->configure(cfg);
    }
    

    // std::cout << "asd " << std::endl;
    


    // various software filters ...
    // one HF for ROI,  two LF for ROI finding
    // one HF for charge
    // one HF for hit
    // two HF filters for wire dimension 
    
    
    int nrebin = 6;


    ExecMon em("starting");
    std::string url = argv[1];

    XinFileIterator fs(url.c_str());

    cerr <<  em("loading rootfiles") << endl;

    IFrame::pointer frame = fs.frame();

    //    cerr << em("fill the frame") << endl;

    fs.clear();
    
    cerr <<  em("close the file") << endl;
    
    
    SigProc::OmnibusSigProc bus;
    bus.configure(bus.default_configuration());


    IFrame::pointer frame_decon;
    
    cerr << em("Do deconvolution") << endl;
    bus(frame, frame_decon);
    cerr << em(" ... done") << endl;
    
    save_into_file(url.c_str(),frame_decon,nrebin);
    
    
}
