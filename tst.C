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
  Container<TrackParCov,int> cnt;

  // set user info (data identifier etc.)
  cnt.setUserInfo(0xdeadbeaf);
  
  float b = 5.0f; // B-field

  for (int i=0;i<10;i++) {
    //
    TrackParCov tr0(0.f,0.f,arp,arc);
    tr0.PropagateTo(float(1+(i*10)%100),b);
    cnt.push_back(tr0);
    //
    TrackParCov* tr1 = cnt.emplace_back(0.,1., arp,arc);
    tr1->PropagateTo(float(5+(i*10)%100),b);
    //
  }
  
}
