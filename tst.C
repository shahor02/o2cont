//#include "TrackPar.h"
#include "DetectorsBase/Track.h"
#include "ContVec.h"
#include <TFile.h>
#include <fstream>


void  writeToBinFile(const char* ptr, int nbytes, const char* fname="containerTest.bin");
char* readFromBinFile(int& nread, const char* fname="containerTest.bin");

using namespace o2::Base::Track;

void tst()
{
  std::array<float,5> arp={0.,0.,0.1,0.1,1};
  std::array<float,15> arc={
    0.01,
    0.,  0.01,
    0.,  0.  ,0.01,
    0.,  0.  ,0.  ,0.01,
    0.,  0.  ,0.  ,0.  ,0.01
  };

  // container for objects of type TrackParCov
  // and user info object type int
  ContVec<o2::Base::Track::TrackParCov,int> cnt;

  // set user info (data identifier etc.)
  cnt.setUserInfo(0xdeadbeaf);
  
  float b = 5.0f; // B-field

  // here we create some track in the container
  for (int i=0;i<10;i++) {
    //
    TrackParCov tr0(0.f,0.f,arp,arc);
    tr0.PropagateTo(float(1+(i*10)%100),b);
    cnt.push_back(tr0);      // track copy 
    //
    TrackParCov* tr1 = cnt.emplace_back(0.,1., arp,arc); // or create in the array directly
    tr1->PropagateTo(float(5+(i*10)%100),b); 
    //
  }

  // store container in a root file as single object
  TFile* flout = TFile::Open("contobj.root","recreate");
  cnt.Write("cntw");
  flout->Close();
  delete flout;
  //
  // read back
  TFile* flin = TFile::Open("contobj.root");
  //  ContVec<TrackParCov,int>
  auto cntroot = reinterpret_cast<ContVec<o2::Base::Track::TrackParCov,int>*>(flin->Get("cntw"));
  flin->Close();
  delete flin;
  //
  //======================================================================
  // Test transfer of raw pointers, makes sense only for plain data objects
  //
  int nb = 0;
  char* pntr = 0;
  
  // Write to bin file the raw pointer preserving the original object
  writeToBinFile(cnt.getPtr(),cnt.sizeInBytes(),"containerTest0.bin");
  //
  // Read: recreate containers from raw pointer
  pntr = readFromBinFile(nb,"containerTest0.bin");
  //  
  // "false" indicates that the pntr is currently not managed, so that the new container does not need
  // to create new buffer but just take ownership of the buffer pointed by pntr
  // 
  // nb is passed just for consistency check
  ContVec<o2::Base::Track::TrackParCov,int> cntb0(pntr,false,nb); 

  
  // Write to bin file the raw pointer resetting the original object
  nb = cntroot->sizeInBytes();
  std::unique_ptr<char[]> tmpPtr( cntroot->release() ); // this will reset cntroot
  writeToBinFile(tmpPtr.get(),nb,"containerTest1.bin");
  delete[] tmpPtr.release(); // just to dispose unnecessary buffer
  //
  // Read: recreate containers from raw pointer
  tmpPtr.reset( readFromBinFile(nb,"containerTest1.bin") );
  //  
  // "true" indicates that the raw pointer is currently managed, so that the new container must 
  // create new buffer and copy the content
  // -1 (default) indicates no request for buffer size consistency check
  ContVec<o2::Base::Track::TrackParCov,int> cntb1(tmpPtr.get(),true,-1);
  delete[] tmpPtr.release(); // just to dispose unnecessary buffer

  //======================================================================
  // Test output to TTree
  //
  // For debug purposes (also until the messaging is fully operational) we need
  // to be able to store objects as vectors in the root tree 
  //
  TFile* fltree = TFile::Open("contTree.root","recreate");
  TTree* tree = new TTree("tstTree","testTree");
  for (int i=0;i<100;i++) {
    // modifiy slightly the tracks to simulate new event
    for (int j=cnt.size();j--;) {
      auto trc = cnt[j];
      trc->PropagateTo( trc->GetX()+(i*10+j)%20 ,b);
    }
    cnt.AddToTree(tree,"Tracks");
    tree->Fill();
    //
  }
  tree->Write();
  delete tree;
  fltree->Close();
  delete fltree;
  //
}


//____________________________________________
void  writeToBinFile(const char* ptr, int nbytes, const char* fname)
{
  // write char buffer to binary file
  ofstream file(fname, ios::out|ios::binary|ios::trunc);
  file.write(ptr,nbytes);
  file.close();
}

//____________________________________________
char* readFromBinFile(int &nread, const char* fname)
{
  // read char buffer from binary file
  nread = 0;
  ifstream file(fname, ios::in|ios::binary|ios::ate);
  nread = (int)file.tellg();
  unique_ptr<char[]> buff(new char[nread]);
  file.seekg (0, ios::beg);
  file.read(buff.get(),nread);
  file.close();
  return buff.release();
}

