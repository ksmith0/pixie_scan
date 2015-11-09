/** \file LaBr3TestProcessor.cpp
 *\brief A Labr3Test processor class that can be used to build your own.
 *\author S. V. Paulauskas
 *\date October 26, 2014
 */
#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "HighResTimingData.hpp"
#include "RawEvent.hpp"
#include "LaBr3TestProcessor.hpp"

namespace dammIds {
    namespace labr3test {
      const int OFFSET = 7990;
      const int RANGE =9;
        const int D_TDIFF  = 0; //!< ID for the energy of the template detector
        const int DD_PP = 1; //!< Energy Labr3Test vs. Energy Pulser
        const int DD_AMPMAP = 2; //!< Energy vs. Sample Number
			const int D_ENERGY = 3;
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::labr3test;

Labr3TestProcessor::Labr3TestProcessor():
    EventProcessor(dammIds::labr3test::OFFSET, dammIds::labr3test::RANGE,
                   "labr3") {
    associatedTypes.insert("labr3");
}

void Labr3TestProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_TDIFF, SE, "Time Difference");
    DeclareHistogram2D(DD_PP, SE, SE,"Phase - Phase");
    DeclareHistogram2D(DD_AMPMAP, SE, SE, "AmplitudeMap");
    DeclareHistogram1D(D_ENERGY, SE, "Energy");

	file = new TFile("output.root","RECREATE");
	tree = new TTree("labr3","labr3");
	tree->Branch("en_labr",&energies1);
	tree->Branch("en_start",&energies2);
	tree->Branch("tdiff",&tdiff);
	tree->Branch("phase_labr",&phase1);
	tree->Branch("phase_start",&phase2);
}

bool Labr3TestProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return(false);

    static const vector<ChanEvent*> & labr3Events =
        event.GetSummary("labr3")->GetList();
    
    for(vector<ChanEvent*>::const_iterator it = labr3Events.begin();
        it != labr3Events.end(); it++) {
        unsigned int location = (*it)->GetChanID().GetLocation();
	// if(location == 0)
	//     plot(D_ENERGY, (*it)->GetEnergy());
    }    
    return(true);
}

bool Labr3TestProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);

    static const vector<ChanEvent*> & labr3Events =
        event.GetSummary("labr3")->GetList();
    static const vector<ChanEvent*> & tvandleStarts =
        event.GetSummary("tvandle:single:start")->GetList();
    static const vector<ChanEvent*> & labr3_r6231_start =
        event.GetSummary("labr3:r6231_100:start")->GetList();
    static const vector<ChanEvent*> & labr3_r7724_start =
        event.GetSummary("labr3:r7724_100:start")->GetList();
	vector<ChanEvent*> starts;
		starts.insert(starts.end(),labr3_r7724_start.begin(),labr3_r7724_start.end());
		starts.insert(starts.end(),labr3_r6231_start.begin(),labr3_r6231_start.end());
		starts.insert(starts.end(),tvandleStarts.begin(),tvandleStarts.end());

    double timeRes  = 50; //20 ps/bin
    double timeOff  = 2000.;
    double phaseX   = -18000.;

	 for(vector<ChanEvent*>::const_iterator it = labr3Events.begin();
			 it != labr3Events.end(); it++) {
		 HighResTimingData labr3((*it));

		energies1.clear();
		energies2.clear();
		phase1.clear();
		phase2.clear();
		tdiff.clear();


		 if(labr3.GetIsValidData())
			 for(Trace::const_iterator it = labr3.GetTrace()->begin();
					 it != labr3.GetTrace()->end(); it++)
				 plot(DD_AMPMAP, int(it-labr3.GetTrace()->begin()), *it);

		 for(vector<ChanEvent*>::const_iterator itA = starts.begin();
				 itA != starts.end(); itA++) {
			 HighResTimingData start((*itA));

			 if(tvandle.GetIsValidData() && labr3.GetIsValidData()) {
	//			 cout << (labr3.GetHighResTime() - tvandle.GetHighResTime())*
	//				 timeRes+timeOff << " " << labr3.GetPhase()*timeRes << " " << 
	//				 tvandle.GetPhase()*timeRes+phaseX << endl;

				 energies1.push_back(labr3.GetPixieEnergy());
				 energies2.push_back(start.GetPixieEnergy());
				phase1.push_back(labr3.GetPhase());
				phase2.push_back(start.GetPhase());
				 tdiff.push_back(labr3.GetHighResTime() - start.GetHighResTime());


				 plot(D_TDIFF, (labr3.GetHighResTime() - start.GetHighResTime())*
						 timeRes+timeOff);
				 plot(DD_PP, labr3.GetPhase()*timeRes+phaseX, 
						 start.GetPhase()*timeRes+phaseX);
			 }
		 }
		tree->Fill();
	 }
	 return(true);
}
