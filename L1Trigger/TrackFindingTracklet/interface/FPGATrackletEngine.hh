//This class implementes the tracklet engine
#ifndef FPGATRACKLETENGINE_H
#define FPGATRACKLETENGINE_H

#include "FPGAProcessBase.hh"


using namespace std;

class FPGATrackletEngine:public FPGAProcessBase{

public:

  FPGATrackletEngine(string name, unsigned int iSector):
    FPGAProcessBase(name,iSector){
    double dphi=two_pi/NSector;
    phimin_=iSector*dphi;
    phimax_=phimin_+dphi;
    if (phimin_>0.5*two_pi) phimin_-=two_pi;
    if (phimax_>0.5*two_pi) phimax_-=two_pi;
    if (phimin_>phimax_)  phimin_-=two_pi;
    //cout << "phimin_ phimax_ "<<phimin_<<" "<<phimax_<<endl;
    assert(phimax_>phimin_);
    stubpairs_=0;
    innervmstubs_=0;
    outervmstubs_=0;
    //table_=0;
    layer1_=0;
    layer2_=0;
    disk1_=0;
    disk2_=0;
    dct1_=0;
    dct2_=0;
    phi1_=0;
    phi2_=0;
    z1_=0;
    z2_=0;
    r1_=0;
    r2_=0;
    if (name[3]=='L') {
      layer1_=name[4]-'0';
      z1_=name[12]-'0';
    }
    if (name[3]=='D') {
      disk1_=name[4]-'0';
      r1_=name[12]-'0';
    }
    if (name[11]=='L') {
      layer2_=name[12]-'0';
      z2_=name[23]-'0';
    }
    if (name[11]=='D') {
      disk2_=name[12]-'0';
      r2_=name[23]-'0';
    }
    if (name[12]=='L') {
      layer2_=name[13]-'0';
      z2_=name[23]-'0';
    }
    if (name[12]=='D') {
      disk2_=name[13]-'0';
      r2_=name[23]-'0';
    }
    
    phi1_=name[10]-'0';
    phi2_=name[21]-'0';

    dct1_=name[6]-'0';
    dct2_=name[17]-'0';


  }

  void addOutput(FPGAMemoryBase* memory,string output){
    if (writetrace) {
      cout << "In "<<name_<<" adding output to "<<memory->getName()
	   << " to output "<<output<<endl;
    }
    if (output=="stubpairout") {
      FPGAStubPairs* tmp=dynamic_cast<FPGAStubPairs*>(memory);
      assert(tmp!=0);
      stubpairs_=tmp;
      return;
    }
    assert(0);
  }

  void addInput(FPGAMemoryBase* memory,string input){
    if (writetrace) {
      cout << "In "<<name_<<" adding input from "<<memory->getName()
	   << " to input "<<input<<endl;
    }
    if (input=="innervmstubin") {
      FPGAVMStubsTE* tmp=dynamic_cast<FPGAVMStubsTE*>(memory);
      assert(tmp!=0);
      innervmstubs_=tmp;
      setVMPhiBin();
      return;
    }
    if (input=="outervmstubin") {
      FPGAVMStubsTE* tmp=dynamic_cast<FPGAVMStubsTE*>(memory);
      assert(tmp!=0);
      outervmstubs_=tmp;
      setVMPhiBin();
      return;
    }
    cout << "Could not find input : "<<input<<endl;
    assert(0);
  }

  void execute() {

    
    if (!((doL1L2&&(layer1_==1)&&(layer2_==2))||
	  (doL3L4&&(layer1_==3)&&(layer2_==4))||
	  (doL5L6&&(layer1_==5)&&(layer2_==6))||
	  (doD1D2&&(disk1_==1)&&(disk2_==2))||
	  (doD3D4&&(disk1_==3)&&(disk2_==4))||
	  (doL1D1&&(disk1_==1)&&(layer2_==1))||
	  (doL2D1&&(disk1_==1)&&(layer2_==2)))) return;


    unsigned int countall=0;
    unsigned int countpass=0;


    assert(innervmstubs_!=0);
    assert(outervmstubs_!=0);

    if (disk1_==1 && (layer2_==1 || layer2_==2) ) {

      for(unsigned int i=0;i<outervmstubs_->nStubs();i++){
	std::pair<FPGAStub*,L1TStub*> outerstub=outervmstubs_->getStub(i);

	if (debug1) cout << "Have overlap stub in layer = "<<outerstub.first->layer().value()+1<<" disk = "<<outerstub.first->disk().value()<<endl;
	
	int lookupbits=outerstub.first->getVMBits().value();
        int rdiffmax=(lookupbits>>7);	
	int newbin=(lookupbits&127);
	int bin=newbin/8;

	int rbinfirst=newbin&7;

	int start=(bin>>1);
	int last=start+(bin&1);
	if (last==8) {
	  cout << "Warning last==8 start="<<start<<endl;
	  last=start;
	}
	for(int ibin=start;ibin<=last;ibin++) {

	  for(unsigned int j=0;j<innervmstubs_->nStubsBinned(ibin);j++){
	    countall++;
	    std::pair<FPGAStub*,L1TStub*> innerstub=innervmstubs_->getStubBinned(ibin,j);
	    int rbin=(innerstub.first->getVMBits().value()&7);
	    if (start!=ibin) rbin+=8;
	    if (rbin<rbinfirst) continue;
	    if (rbin-rbinfirst>rdiffmax) continue;
	    stubpairs_->addStubPair(innerstub,outerstub);
	    countpass++;
	  }
	}

      }
    } else {
    
      for(unsigned int i=0;i<innervmstubs_->nStubs();i++){
	std::pair<FPGAStub*,L1TStub*> innerstub=innervmstubs_->getStub(i);
	if (debug1) {
	  cout << "In "<<getName()<<" have inner stub"<<endl;
	}
	
	if ((layer1_==1 && layer2_==2)||
	    (layer1_==3 && layer2_==4)||
	    (layer1_==5 && layer2_==6)) {
	  

	  if (debug1) cout << "Layer-layer pair" <<endl;
	  
	  int lookupbits=innerstub.first->getVMBits().value();
	  int zdiffmax=(lookupbits>>7);	
	  int newbin=(lookupbits&127);
	  int bin=newbin/8;
	  
	  int zbinfirst=newbin&7;
	
	  int start=(bin>>1);
	  int last=start+(bin&1);
	  for(int ibin=start;ibin<=last;ibin++) {
	    for(unsigned int j=0;j<outervmstubs_->nStubsBinned(ibin);j++){
	      countall++;
	      std::pair<FPGAStub*,L1TStub*> outerstub=outervmstubs_->getStubBinned(ibin,j);
              
	      int zbin=(outerstub.first->getVMBits().value()&7);
	      if (start!=ibin) zbin+=8;
	      if (zbin<zbinfirst) continue;
	      if (zbin-zbinfirst>zdiffmax) continue;

	      int innerphibits=1;
	      int outerphibits=2;

	      int iphiinnerbin=innerstub.first->iphivmFineBins(5,innerphibits);
	      int iphiouterbin=outerstub.first->iphivmFineBins(4,outerphibits);


	      
	      int index = (iphiinnerbin<<outerphibits)+iphiouterbin;

	      assert(index<(int)phitable_.size());		
	      if (!phitable_[index]) continue;

              FPGAWord innerbend=innerstub.first->bend();
              FPGAWord outerbend=outerstub.first->bend();
              
              int ptinnerindex=(index<<innerbend.nbits())+innerbend.value();
              int ptouterindex=(index<<outerbend.nbits())+outerbend.value();

	      if (!pttableinner_[ptinnerindex]) continue;
	      if (!pttableouter_[ptouterindex]) continue;
	      
	      if (debug1) cout << "Adding layer-layer pair in " <<getName()<<endl;
	      stubpairs_->addStubPair(innerstub,outerstub);

	      countpass++;
	    }
	  }
	
	} else if ((disk1_==1 && disk2_==2)||
		   (disk1_==3 && disk2_==4)) {
	  
	  if (debug1) cout << "Disk-disk pair" <<endl;
	  
	  int lookupbits=innerstub.first->getVMBits().value();
	  bool negdisk=innerstub.first->disk().value()<0;
	  int rdiffmax=(lookupbits>>6);	
	  int newbin=(lookupbits&63);
	  int bin=newbin/8;
	  
	  int rbinfirst=newbin&7;
	  
	  int start=(bin>>1);
	  if (negdisk) start+=4;
	  int last=start+(bin&1);
	  for(int ibin=start;ibin<=last;ibin++) {
	    for(unsigned int j=0;j<outervmstubs_->nStubsBinned(ibin);j++){
	      countall++;
	      std::pair<FPGAStub*,L1TStub*> outerstub=outervmstubs_->getStubBinned(ibin,j);
	      int rbin=(outerstub.first->getVMBits().value()&7);
	      if (start!=ibin) rbin+=8;
	      if (rbin<rbinfirst) continue;
	      if (rbin-rbinfirst>rdiffmax) continue;

	      stubpairs_->addStubPair(innerstub,outerstub);
	      countpass++;
	
	    }
	  }
	}
      }
      
    }
  
    if (countall>5000) {
      cout << "In FPGATrackletEngine::execute : "<<getName()
	   <<" "<<innervmstubs_->nStubs()
	   <<" "<<outervmstubs_->nStubs()
	   <<" "<<countall<<" "<<countpass
	   <<endl;
      for(unsigned int i=0;i<innervmstubs_->nStubs();i++){
	std::pair<FPGAStub*,L1TStub*> innerstub=innervmstubs_->getStub(i);
	cout << "In FPGATrackletEngine::execute inner stub : "
	     << innerstub.second->r()<<" "
	     << innerstub.second->phi()<<" "
	     << innerstub.second->r()*innerstub.second->phi()<<" "
	     << innerstub.second->z()<<endl;
      }
      for(unsigned int i=0;i<outervmstubs_->nStubs();i++){
	std::pair<FPGAStub*,L1TStub*> outerstub=outervmstubs_->getStub(i);
	cout << "In FPGATrackletEngine::execute outer stub : "
	     << outerstub.second->r()<<" "
	     << outerstub.second->phi()<<" "
	     << outerstub.second->r()*outerstub.second->phi()<<" "
	     << outerstub.second->z()<<endl;
      }
      
    }
      
    if (writeTE) {
      static ofstream out("trackletengine.txt");
      out << getName()<<" "<<countall<<" "<<countpass<<endl;
    }
      
  }

  void setVMPhiBin() {
    if (innervmstubs_==0 || outervmstubs_==0 ) return;

    innervmstubs_->setother(outervmstubs_);
    outervmstubs_->setother(innervmstubs_);

    
    if ((layer1_==1 && layer2_==2)||
	(layer1_==3 && layer2_==4)||
	(layer1_==5 && layer2_==6)){

      int innerphibits=1;
      int outerphibits=2;

      int innerphibins=(1<<innerphibits);
      int outerphibins=(1<<outerphibits);

      double innerphimin, innerphimax;
      innervmstubs_->getPhiRange(innerphimin,innerphimax);
      double rinner=rmean[layer1_-1];
      
      double outerphimin, outerphimax;
      outervmstubs_->getPhiRange(outerphimin,outerphimax);
      double router=rmean[layer2_-1];

      double phiinner[2];
      double phiouter[2];
	
      for (int iphiinnerbin=0;iphiinnerbin<innerphibins;iphiinnerbin++){
	phiinner[0]=innerphimin+iphiinnerbin*(innerphimax-innerphimin)/innerphibins;
	phiinner[1]=innerphimin+(iphiinnerbin+1)*(innerphimax-innerphimin)/innerphibins;
	for (int iphiouterbin=0;iphiouterbin<outerphibins;iphiouterbin++){
	  phiouter[0]=outerphimin+iphiouterbin*(outerphimax-outerphimin)/outerphibins;
	  phiouter[1]=outerphimin+(iphiouterbin+1)*(outerphimax-outerphimin)/outerphibins;

          double bendinnermin=20.0;
          double bendinnermax=-20.0;
          double bendoutermin=20.0;
          double bendoutermax=-20.0;
          double rinvmin=1.0; 
          for(int i1=0;i1<2;i1++) {
            for(int i2=0;i2<2;i2++) {
              double rinv1=rinv(phiinner[i1],phiouter[i2],rinner,router);
              double abendinner=bend(rinner,-rinv1); //FIXME
              double abendouter=bend(router,-rinv1);
              //if (iSector_==0&&layer1_==1) {
              //cout << "rin rinv1 bend : "<<rin[i3]<<" "<<rinv1<<" "<<abendinner<<endl;
              //}
              if (abendinner<bendinnermin) bendinnermin=abendinner;
              if (abendinner>bendinnermax) bendinnermax=abendinner;
              if (abendouter<bendoutermin) bendoutermin=abendouter;
              if (abendouter>bendoutermax) bendoutermax=abendouter;
              if (fabs(rinv1)<rinvmin) {
                rinvmin=fabs(rinv1);
              }
		      
            }
          }

          phitable_.push_back(rinvmin<rinvcut);
          double bendinner=0.5*(bendinnermin+bendinnermax);
          double bendouter=0.5*(bendoutermin+bendoutermax);
          
          for(int ibend=0;ibend<32;ibend++) {
            double bend=(ibend-15.0)/2.0; 

            pttableinner_.push_back(fabs(bend-bendinner)<1.5);
            pttableouter_.push_back(fabs(bend-bendouter)<1.5);
            
          }

        }
      }

      if (iSector_==0&&writeTETables) writeTETable();
      
    }

  }

  double rinv(double phi1, double phi2,double r1, double r2){

    assert(r2>r1);

    double dphi=phi2-phi1;
    double dr=r2-r1;
    
    return 2.0*sin(dphi)/dr/sqrt(1.0+2*r1*r2*(1.0-cos(dphi))/(dr*dr));
    
  }

  double bend(double r, double rinv) {

    double dr=0.18;
    
    double delta=r*dr*0.5*rinv;

    double bend=-delta/0.009;
    if (r<55.0) bend=-delta/0.01;

    return bend;
    
  }

  void writeTETable() {

    ofstream outptcut;
    outptcut.open(getName()+"_ptcut.txt");
    for(unsigned int i=0;i<phitable_.size();i++){
      outptcut << i << " "  << phitable_[i]<<endl;
    }
    outptcut.close();

    ofstream outstubptinnercut;
    outstubptinnercut.open(getName()+"_stubptinnercut.txt");
    for(unsigned int i=0;i<pttableinner_.size();i++){
      outstubptinnercut << i << " "  << pttableinner_[i]<<endl;
    }
    outstubptinnercut.close();
    
    ofstream outstubptoutercut;
    outstubptoutercut.open(getName()+"_stubptoutercut.txt");
    for(unsigned int i=0;i<pttableouter_.size();i++){
      outstubptoutercut << i << " "  << pttableouter_[i]<<endl;
    }
    outstubptoutercut.close();

    
  }
  
  private:

    double phimax_;
    double phimin_;

    int layer1_;
    int layer2_;
    int disk1_;
    int disk2_;
    int dct1_;
    int dct2_;
    int phi1_;
    int phi2_;
    int z1_;
    int z2_;
    int r1_;
    int r2_;
  
    FPGAVMStubsTE* innervmstubs_;
    FPGAVMStubsTE* outervmstubs_;
    
    FPGAStubPairs* stubpairs_;

    vector<bool> phitable_;
  //vector<double> bendtableinner_;
  // vector<double> bendtableouter_;
    vector<bool> pttableinner_;
    vector<bool> pttableouter_;

    
  };

#endif
