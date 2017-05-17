#ifdef __CLING__


#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

//#pragma link C++ class TrackParBase+;
//#pragma link C++ class TrackParCov+;
//#pragma link C++ class TrackPar+;
#pragma link C++ class Container<o2::Base::Track::TrackParCov,int>-;
#pragma link C++ class Container<double,int>-;
#pragma link C++ class vector<o2::Base::Track::TrackParCov>+;

#endif
