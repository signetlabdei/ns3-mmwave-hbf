/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2019 University of Padova, Dep. of Information Engineering, SIGNET lab.
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation;
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "ns3/mmwave-beamforming-model.h"
#include "ns3/antenna-array-basic-model.h"
#include "ns3/antenna-array-model.h"
#include "ns3/mobility-model.h"
#include "ns3/pointer.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include <ns3/simulator.h>

#include "ns3/three-gpp-spectrum-propagation-loss-model.h"

#include <complex>

#include <iostream>
#include <fstream>

namespace ns3 {

namespace mmwave {

NS_LOG_COMPONENT_DEFINE ("MmWaveBeamformingModel");

/*----------------------------------------------------------------------------*/

NS_OBJECT_ENSURE_REGISTERED (MmWaveBeamformingModel);

TypeId
MmWaveBeamformingModel::GetTypeId ()
{
  static TypeId
    tid =
    TypeId ("ns3::MmWaveBeamformingModel")
    .SetParent<Object> ()
    .AddAttribute ("AntennaArray",
                   "Poiter to the antenna array",
                   PointerValue (),
                   MakePointerAccessor (&MmWaveBeamformingModel::SetAntenna,
                                        &MmWaveBeamformingModel::GetAntenna),
                   MakePointerChecker<AntennaArrayBasicModel> ())
  ;
  return tid;
}

MmWaveBeamformingModel::MmWaveBeamformingModel ()
{
  NS_LOG_FUNCTION (this);
}

MmWaveBeamformingModel::~MmWaveBeamformingModel ()
{

}

Ptr<AntennaArrayBasicModel>
MmWaveBeamformingModel::GetAntenna () const
{
  NS_LOG_FUNCTION (this);

  return m_antenna;
}

void
MmWaveBeamformingModel::SetAntenna (Ptr<AntennaArrayBasicModel> antenna)
{
  NS_LOG_FUNCTION (this);

  m_antenna = antenna;
}


void
MmWaveBeamformingModel::SetSpectrumPropagationLossModel (Ptr<SpectrumPropagationLossModel> spectrumPropagationLossModel)
{

  NS_LOG_FUNCTION (this);

  m_spectrumPropagationLossModel = spectrumPropagationLossModel;
}

/*----------------------------------------------------------------------------*/

NS_OBJECT_ENSURE_REGISTERED (MmWaveDftBeamforming);

TypeId
MmWaveDftBeamforming::GetTypeId ()
{
  static TypeId
    tid =
    TypeId ("ns3::MmWaveDftBeamforming")
    .SetParent<MmWaveBeamformingModel> ()
    .AddConstructor<MmWaveDftBeamforming> ()
    .AddAttribute ("MobilityModel",
                   "Poiter to the MobilityModel associated with this device",
                   PointerValue (),
                   MakePointerAccessor (&MmWaveDftBeamforming::m_mobility),
                   MakePointerChecker<MobilityModel> ())
  ;
  return tid;
}

MmWaveDftBeamforming::MmWaveDftBeamforming ()
{
  NS_LOG_FUNCTION (this);
}

MmWaveDftBeamforming::~MmWaveDftBeamforming ()
{

}

AntennaArrayBasicModel::BeamformingVector
MmWaveDftBeamforming::DoDesignBeamformingVectorForDevice (Ptr<NetDevice> otherDevice)
{
  // retrieve the position of the two devices
  Vector aPos = m_mobility->GetPosition ();
  Vector bPos = otherDevice->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();

  // compute the azimuth and the elevation angles
  Angles completeAngle (bPos,aPos);

  double posX = bPos.x - aPos.x;
  double phiAngle = atan ((bPos.y - aPos.y) / posX);

  if (posX < 0)
    {
      phiAngle = phiAngle + M_PI;
    }
  if (phiAngle < 0)
    {
      phiAngle = phiAngle + 2 * M_PI;
    }

  double hAngleRadian = fmod ((phiAngle + M_PI),2 * M_PI - M_PI); // the azimuth angle
  double vAngleRadian = completeAngle.theta; // the elevation angle

  // retrieve the number of antenna elements
  uint16_t antennaNum [2];
  antennaNum[0] = m_antenna->GetAntennaNumDim1 ();
  antennaNum[1] = m_antenna->GetAntennaNumDim2 ();
  uint32_t totNoArrayElements = antennaNum[0]*antennaNum[1];

  // the total power is divided equally among the antenna elements
  double power = 1 / sqrt (totNoArrayElements);

  AntennaArrayBasicModel::BeamformingVector newBfParam;

  // compute the antenna weights
  for (uint32_t ind = 0; ind < totNoArrayElements; ind++)
    {
      Vector loc = m_antenna->GetAntennaLocation (ind);
      double phase = -2 * M_PI * (sin (vAngleRadian) * cos (hAngleRadian) * loc.x
	  + sin (vAngleRadian) * sin (hAngleRadian) * loc.y
	  + cos (vAngleRadian) * loc.z);
      newBfParam.first.push_back (exp (std::complex<double> (0, phase)) * power);
    }

  // bId = 0; // TODO how to set the bid?
  //TODO [fgomez] consider this beamID proposal, the beam is identified by the pair of IDs of the transmitter and receiver
  uint32_t minId = std::min (m_mobility->GetObject<Node> ()->GetId (), otherDevice->GetNode ()->GetId ());
  uint32_t maxId = std::max (m_mobility->GetObject<Node> ()->GetId (), otherDevice->GetNode ()->GetId ());
  newBfParam.second = GetKey(minId,maxId); //in this model, beam ID is the pair of devices
  //TODO this is the same Cantor function as in ThreeGppChannel::GetKey (minId, maxId), consider unifying

  //SAVE THE NEW BEAM HERE
  // we can make modifications in beamID below without changing map key here
  uint32_t beamKey = GetKey(m_mobility->GetObject<Node> ()->GetId (),otherDevice->GetNode ()->GetId ());

  //update the cache with a new value
  //TODO do we need to garbage collect the old cache value here because it was a pointer?
  Ptr<BFVectorCacheEntry> pCacheValue = Create<BFVectorCacheEntry> ();
  pCacheValue->m_myPos = aPos;
  pCacheValue->m_otherPos = bPos;
  pCacheValue->m_beamId = newBfParam.second;
  pCacheValue->m_antennaWeights =newBfParam.first;

  m_vectorCache[beamKey] = pCacheValue;

  return( newBfParam );
}

bool
MmWaveDftBeamforming::CheckBfCacheExpiration(Ptr<NetDevice> otherDevice, Ptr<BFVectorCacheEntry> pCacheValue)
{
  Vector aPos = m_mobility->GetPosition ();
  Vector bPos = otherDevice->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
  	return(
  	    ( aPos.x != pCacheValue->m_myPos.x ) ||
  	    ( aPos.y != pCacheValue->m_myPos.y ) ||
	    ( aPos.z != pCacheValue->m_myPos.z ) ||
	    ( bPos.x != pCacheValue->m_otherPos.x ) ||
	    ( bPos.y != pCacheValue->m_otherPos.y ) ||
	    ( bPos.z != pCacheValue->m_otherPos.z )
	    );
}

void
MmWaveDftBeamforming::SetBeamformingVectorForDevice (Ptr<NetDevice> otherDevice , uint8_t layerInd)
{
  NS_LOG_FUNCTION (this);

  AntennaArrayBasicModel::complexVector_t antennaWeights;
  AntennaArrayBasicModel::BeamId bId;

  NS_ASSERT_MSG (otherDevice->GetNode (), "the device " << otherDevice << " is not associated to a node");
  NS_ASSERT_MSG (otherDevice->GetNode ()->GetObject<MobilityModel> (), "the device " << otherDevice << " has not a mobility model");

  bool update = false;
  bool notFound = false;

  // we can make modifications in beamID below without changing map key here
  uint32_t beamKey = GetKey(m_mobility->GetObject<Node> ()->GetId (),otherDevice->GetNode ()->GetId ());

  std::map< uint32_t, Ptr<BFVectorCacheEntry> >::iterator itVectorCache = m_vectorCache.find(beamKey);
  Ptr<BFVectorCacheEntry> pCacheValue;
  if ( itVectorCache != m_vectorCache.end() )
    {
      NS_LOG_DEBUG ("found a beam in the map");
      pCacheValue = itVectorCache->second;
      update = CheckBfCacheExpiration( otherDevice,  pCacheValue);
    }
  else
  {
    NS_LOG_DEBUG ("beam NOT found");
    notFound = true;
  }

  if ( notFound || update )
    {
      NS_LOG_DEBUG ("Creating a new beam");
      AntennaArrayBasicModel::BeamformingVector newBfStruct = DoDesignBeamformingVectorForDevice (otherDevice);
      antennaWeights = newBfStruct.first;
      bId = newBfStruct.second;

      //DoDesignBeamformingVectorForDevice must store the vector in cache
    }
  else
    { //if we enter this segment of code pCacheValue has been pointed to a valid cache entry, which we read
      bId = pCacheValue->m_beamId;
      antennaWeights = pCacheValue->m_antennaWeights;
    }

  // configure the antenna to use the new beamforming vector
  Ptr<AntennaArrayModel> castAntenna = DynamicCast<AntennaArrayModel>(m_antenna);
  if ( castAntenna != 0 )
    {
      castAntenna->SetBeamformingVectorMultilayers (antennaWeights, bId, otherDevice, layerInd);
    }
  else
    {
      m_antenna->SetBeamformingVector (antennaWeights, bId, otherDevice);
    }
}


/*----------------------------------------------------------------------------*/

NS_OBJECT_ENSURE_REGISTERED (MmWaveFFTCodebookBeamforming);

TypeId
MmWaveFFTCodebookBeamforming::GetTypeId ()
{
  static TypeId
    tid =
    TypeId ("ns3::MmWaveFFTCodebookBeamforming")
    .SetParent<MmWaveDftBeamforming> ()
    .AddConstructor<MmWaveFFTCodebookBeamforming> ()
  ;
  return tid;
}

MmWaveFFTCodebookBeamforming::MmWaveFFTCodebookBeamforming ()
{
  NS_LOG_FUNCTION (this);
}

MmWaveFFTCodebookBeamforming::~MmWaveFFTCodebookBeamforming ()
{

}

void
MmWaveFFTCodebookBeamforming::InPlaceArrayFFT (ComplexArray_t& x)
{
  //This method is a direct transposition of the fft examples provided in https://rosettacode.org/wiki/Fast_Fourier_transform#C.2B.2B
  //The code in this method implementation is under the GNU Free Documentation License 1.2  https://www.gnu.org/licenses/old-licenses/fdl-1.2.html

  const  uint16_t N = x.size();
  if (N <= 1) return;

  // divide
  ComplexArray_t even = x[std::slice(0, N/2, 2)];
  ComplexArray_t  odd = x[std::slice(1, N/2, 2)];

  // conquer
  InPlaceArrayFFT(even);
  InPlaceArrayFFT(odd);

  // combine
  for ( uint16_t k = 0; k < N/2; ++k)
    {
      std::complex<double> t = std::polar(1.0, -2 * PI * k / N) * odd[k];
      x[k    ] = even[k] + t;
      x[k+N/2] = even[k] - t;
    }
}

complex2DVector_t
MmWaveFFTCodebookBeamforming::MmseCholesky (complex2DVector_t matrixH)
{
  //This method obtains the cholesky decomposition of the positive definite hermitian matrix M=(H'H+I),
  //where we have left the input matrixH in the format H
  //This method is a C++ adaptation of the cholesky decompositoin examples provided in https://rosettacode.org/wiki/Cholesky_decomposition#C
  //The code in this section is NOT a direct copy of the site

  complex2DVector_t Llower;
  uint16_t Msize = matrixH.at(0).size();
  for ( uint16_t krow = 0 ; krow < Msize ; krow ++)
    {//initialize the result matrix
      complexVector_t newLRow(Msize,0.0);
      Llower.push_back(newLRow);
    }

  for ( uint16_t kcol = 0 ; kcol < Msize ; kcol ++)
    {
      for ( uint16_t kdiag = 0 ; kdiag < (kcol + 1) ; kdiag ++) //equality reached in loop end
        {
	  //first we build the necessary coefficient of Hermitian matrix M(kcol,kdiag) = H'H+I
	  std::complex<double> sum = ( kcol == kdiag )? 1 : 0;
	  for ( uint16_t sumiter = 0 ; sumiter <  matrixH.size() ; sumiter ++)
	    {
	      sum += std::conj( matrixH.at(sumiter).at(kcol) ) * matrixH.at(sumiter).at(kdiag) ;
	    }
	  //second we apply the negative sumatorium of the normal Cholesky algorithm in the accumulator variable 'sum'
	  for ( uint16_t ksum = 0 ; ksum <  kdiag ; ksum ++) //equality not reached in loop end
	    {
	      sum -= Llower.at(kcol).at(ksum) * std::conj( Llower.at(kdiag).at(ksum) );
	    }
	  //finally we update the matrix
	  Llower.at(kcol).at(kdiag) = ( kcol == kdiag )? sqrt( sum ) : ( sum / Llower.at(kdiag).at(kdiag) );
        }
    }

  return( Llower );
}

complexVector_t
MmWaveFFTCodebookBeamforming::MmseSolve (complex2DVector_t matrixH, complexVector_t v)
{
  //Cholesky linear solver for x of linear system (H'*H+I)x=H'v, returning x=(H'*H+I)^-1H'v
  //note: a linear solver is N times faster than a matrix inversion,in fact
  //      the Cholesky matrix inversion algorithm consists in N linear solvers

  complexVector_t aux1;//aux1=H'v
  for (uint16_t col = 0; col < matrixH.at(0).size() ; col ++)
    {
      std::complex<double> sum=0;
      for (uint16_t row = 0; row < matrixH.size() ; row ++)//col-row indexes inverted here for Hermitian matrix
	{
	  sum += std::conj( matrixH.at(row).at(col) ) * v.at(row);
	}
      aux1.push_back(sum);
    }
  complex2DVector_t matrixL=MmseCholesky(matrixH);//define M = H'H+I, factorization M=L*L' with lower-triangular L

  complexVector_t aux2;// define L'x=aux2, solve L*aux2 = aux1,
  for (uint16_t row = 0; row < matrixL.size() ; row ++)
    {
      std::complex<double> sum=aux1.at(row);
      for (uint16_t col = 0; col < row ; col ++)
	{
	  sum -= aux2.at(col)*matrixL.at(row).at(col);
	}
      aux2.push_back( sum /  matrixL.at(row).at(row) );
    }

  complexVector_t x ( matrixL.size() , 0.0 );// solve L'*x=aux2. We preallocate initialized with zeros because we start writing at the end of the vector
  for (uint16_t revRow = 0; revRow < matrixL.size(); revRow ++ )
    {
      uint16_t row = matrixL.size() -1 - revRow;//we start by the LAST coefficient because L' is upper triangular
      std::complex<double> sum = aux2.at(row);
      for (uint16_t col = row + 1 ; col < matrixL.size() ; col ++)
	{
	  sum -= x.at(col) * std::conj(matrixL.at(col).at(row));//col-row indexes inverted for Hermitian matrix L'
	}
      x[row] = ( sum /  matrixL.at(row).at(row) );
    }

  return( x );
}

void
MmWaveFFTCodebookBeamforming::Channel4DFFT (complex2DVector_t& matrix,Ptr<NetDevice> otherDevice)
{
  // retrieve the number of antenna elements
  uint16_t antennaNum [2];
  antennaNum[0] = m_antenna->GetAntennaNumDim1 ();
  antennaNum[1] = m_antenna->GetAntennaNumDim2 ();
  uint32_t totNoArrayElements = antennaNum[0]*antennaNum[1];
  NS_ASSERT_MSG (totNoArrayElements == matrix.at(0).size(), "Channel matrix size mismatch in 4D FFT method");
  uint16_t otherAntennaNum [2];
  otherAntennaNum[0] = sqrt(matrix.size());//TODO find a way to read dim1 and dim2 from otherDevice to support non-square arrays
  otherAntennaNum[1] = sqrt(matrix.size());
  //step 1; FFT of dim 1 of tx planar array
  for ( uint16_t row = 0; row < matrix.size(); row++)
    {
      complexVector_t::iterator rowItemIterator1 = matrix.at(row).begin();
      complexVector_t::iterator rowItemIterator2 = matrix.at(row).begin() + antennaNum[0];
      for ( uint16_t colSegment = 0; colSegment < antennaNum[1]; colSegment++)
	{
	  complexVector_t subvector(rowItemIterator1,rowItemIterator2);//take a segment from the row we are considering |s1.s1.s1.s1|s2.s2.s2.s2|...
	  ComplexArray_t x (subvector.data(),antennaNum[0]);//convert to valarray for convenient slicing
	  InPlaceArrayFFT(x); //in-place FFT of the segment
	  for ( uint16_t colItem = 0; colItem < antennaNum[0]; colItem++)//TODO can we replace this for with native stl subvector methods?
	    {
	      matrix.at(row).at ( colItem +  antennaNum[0]*colSegment ) = x[colItem] / sqrt( (double )antennaNum[0] ); // replace the segment with its energy-normalized FFT
	    }
	  rowItemIterator1=rowItemIterator2;
	  rowItemIterator2+=+ antennaNum[0];//move iterators to next segment
	}
    }
  //step 2; FFT of dim 2 of tx planar array
    for ( uint16_t row = 0; row < matrix.size(); row++)
      {
	for ( uint16_t colComb = 0; colComb < antennaNum[0]; colComb++)
	  {
	    ComplexArray_t x (antennaNum[1]);//convert to valarray for convenient slicing
	    //take a comb-sample from the row we are considering .c1|c2|c3|c4.c1|c2|c3|c4....
	    for ( uint16_t colItem = 0; colItem < antennaNum[1]; colItem++)//TODO can we replace this for with native stl subvector methods?
	      {
		x[colItem] = matrix.at(row).at ( colComb +  antennaNum[0]*colItem );// read the segment into the subarray
	      }
	    InPlaceArrayFFT(x); //in-place FFT of the segment
	    for ( uint16_t colItem = 0; colItem < antennaNum[1]; colItem++)//TODO can we replace this for with native stl subvector methods?
	      {
		matrix.at(row).at ( colComb +  antennaNum[0]*colItem ) = x[colItem] / sqrt( (double )antennaNum[1] ); // replace the segment with its energy-normalized FFT
	      }
	  }
      }
  //step 3; FFT of dim 1 of rx array
    for ( uint16_t col = 0; col < matrix.at(0).size(); col++)
      {
	for ( uint16_t rowSegment = 0; rowSegment < otherAntennaNum[1]; rowSegment++)
	  {
	    ComplexArray_t x (otherAntennaNum[0]);//convert to valarray for convenient slicing
	    for ( uint16_t rowItem = 0; rowItem < otherAntennaNum[0]; rowItem++)//TODO can we replace this for with native stl subvector methods?
	      {
		x[rowItem] = matrix.at(rowItem + otherAntennaNum[0] * rowSegment).at (col); // read the segment into the subarray
	      }
	    InPlaceArrayFFT(x); //in-place FFT of the segment
	    for ( uint16_t rowItem = 0; rowItem < otherAntennaNum[0]; rowItem++)//TODO can we replace this for with native stl subvector methods?
	      {
		matrix.at(rowItem + otherAntennaNum[0] * rowSegment).at (col) = x[rowItem] / sqrt( (double )otherAntennaNum[0] ); // replace the segment with its energy-normalized FFT
	      }
	  }
      }
  //step 4; FFT of dim 2 of rx array
    for ( uint16_t col = 0; col < matrix.at(0).size(); col++)
      {
	for ( uint16_t rowComb = 0; rowComb < otherAntennaNum[0]; rowComb++)
	  {
	    ComplexArray_t x (otherAntennaNum[1]);//convert to valarray for convenient slicing
	    //take a comb-sample from the row we are considering .c1|c2|c3|c4.c1|c2|c3|c4....
	    for ( uint16_t rowItem = 0; rowItem < otherAntennaNum[1]; rowItem++)//TODO can we replace this for with native stl subvector methods?
	      {
		x[rowItem] = matrix.at ( rowComb +  otherAntennaNum[0]*rowItem ).at(col); // read the segment into the subarray
	      }
	    InPlaceArrayFFT(x); //in-place FFT of the segment
	    for ( uint16_t rowItem = 0; rowItem < otherAntennaNum[1]; rowItem++)//TODO can we replace this for with native stl subvector methods?
	      {
		matrix.at ( rowComb +  otherAntennaNum[0]*rowItem ).at(col) = x[rowItem]  / sqrt( (double )otherAntennaNum[1] ); // replace the segment with its energy-normalized FFT
	      }
	  }
      }
}

bool
MmWaveFFTCodebookBeamforming::CheckBfCacheExpiration(Ptr<NetDevice> otherDevice, Ptr<BFVectorCacheEntry> pCacheValue)
{
  Ptr<CodebookBFVectorCacheEntry> pCacheCasted = DynamicCast<CodebookBFVectorCacheEntry> ( pCacheValue );

  return( ( Simulator::Now ().GetNanoSeconds () - pCacheCasted->m_generatedTime.GetNanoSeconds () ) > 100000 ); //TODO use the channel update time instead of this independent update timer
}


AntennaArrayBasicModel::BeamformingVector
MmWaveFFTCodebookBeamforming::DoDesignBeamformingVectorForDevice (Ptr<NetDevice> otherDevice)
{

  Ptr<ThreeGppSpectrumPropagationLossModel> casted3GPPchan = DynamicCast<ThreeGppSpectrumPropagationLossModel>( m_spectrumPropagationLossModel );

  NS_ASSERT_MSG ( casted3GPPchan != 0, "The spectrum propagation loss model in the channel does not support this BF model");
  //TODO it is theoretically possible to build a 2D channel info using angular samplign with a series of calls to the antenna array radiation patten, but we will not implement this at this time
  complex2DVector_t channelInfo = casted3GPPchan->GetFrequencyFlatChannelMatrixAtDeltaFrequency(m_mobility,otherDevice->GetNode ()->GetObject<MobilityModel> (),0);//TODO put here the deltaFc corresponding to the subcarrier number of the narrowband reference signal in NR
  complex2DVector_t channelInfo_back = channelInfo;
  complex2DVector_t channelInfo_chol = MmseCholesky(channelInfo);
  complexVector_t vTestMmse ( channelInfo_back.size() , 1.0 );//all-ones vector of size NumRxAntenna
  complexVector_t xMmse = MmseSolve(channelInfo_back,vTestMmse);//all-ones vector of size NumRxAntenna
  Channel4DFFT( channelInfo,otherDevice);//in place 4 FFTs for all four dimensions of tx and rx array
  //combined, the four FFTs above transoform channelInfo axes from [rxArrayElem,txArrayElem] into [rxRefAngle,txRefAngle]
  //in an ULA the refAngles correspond to static beams, with angular values asin( (0:Nant-1 /Nant) - Nant/2 )

  uint16_t bestColumn;
  uint16_t bestRow;
  uint16_t totNoArrayElements = channelInfo.at(0).size();
  double bestGain = 0;

  //uncomment these to write to file some channel matrixes and test the FFT and choleski factorization using matlab
    std::stringstream name;
    std::ofstream myfile;
    std::stringstream name2;
    std::ofstream myfile2;
    std::stringstream name3;
    std::ofstream myfile3;
    std::stringstream name4;
    std::ofstream myfile4;
  if ( Simulator::Now ().GetNanoSeconds () == 0 ){
      name<<"fftMatrix"<<m_mobility->GetObject<Node> ()->GetId ()<<"-"<<otherDevice->GetNode ()->GetId ()<<"-"<<Simulator::Now ().GetNanoSeconds ()<<".csv";
      name2<<"chanMatrix"<<m_mobility->GetObject<Node> ()->GetId ()<<"-"<<otherDevice->GetNode ()->GetId ()<<"-"<<Simulator::Now ().GetNanoSeconds ()<<".csv";
      name3<<"cholMatrix"<<m_mobility->GetObject<Node> ()->GetId ()<<"-"<<otherDevice->GetNode ()->GetId ()<<"-"<<Simulator::Now ().GetNanoSeconds ()<<".csv";
      name4<<"mmseVector"<<m_mobility->GetObject<Node> ()->GetId ()<<"-"<<otherDevice->GetNode ()->GetId ()<<"-"<<Simulator::Now ().GetNanoSeconds ()<<".csv";
      myfile.open (name.str());
      myfile2.open (name2.str());
      myfile3.open (name3.str());
      myfile4.open (name4.str());
  }

  for ( uint16_t rxInd=0; rxInd<channelInfo.size(); rxInd++)
    {
      for ( uint16_t txInd=0; txInd<totNoArrayElements; txInd++)
	{
	  double testGain = norm( channelInfo.at(rxInd).at(txInd) ) ;
	  if ( Simulator::Now ().GetNanoSeconds () == 0 ){
	      NS_LOG_DEBUG("In channel matrix for device tx "<< m_mobility->GetObject<Node> ()->GetId () <<
	     	  		       " pointing at device "<< otherDevice->GetNode ()->GetId ()  <<
	     	  		       " channel matrix entiry ("<< rxInd <<","<< txInd <<
	     	  		       ") coef "<< channelInfo.at(rxInd).at(txInd) << " gain "<< testGain <<" best current "<< bestGain);
	      myfile <<channelInfo.at(rxInd).at(txInd).real()<< "," <<channelInfo.at(rxInd).at(txInd).imag()<<",";
	      myfile2 <<channelInfo_back.at(rxInd).at(txInd).real()<< ","<<channelInfo_back.at(rxInd).at(txInd).imag()<<",";
	  }
	  if ( testGain > bestGain)
	    {
	      bestColumn = txInd;
	      bestRow    = rxInd;//this is not used by me, this is the BF vector I assume otherDevice will use if they apply the same bf as me, with transposed matrix
	      bestGain   = testGain;
	    }
	}

      if ( Simulator::Now ().GetNanoSeconds () == 0 ){
	  myfile << "\n";
	  myfile2 << "\n";
      }
    }

  if ( Simulator::Now ().GetNanoSeconds () == 0 ){

      for ( uint16_t rxInd=0; rxInd<totNoArrayElements; rxInd++)
	{
	  for ( uint16_t txInd=0; txInd<totNoArrayElements; txInd++)
	    {
	      myfile3 <<channelInfo_chol.at(rxInd).at(txInd).real()<< ","<<channelInfo_chol.at(rxInd).at(txInd).imag()<<",";
	    }
	  myfile3 << "\n";
	  myfile4 <<xMmse.at(rxInd).real()<< ","<<xMmse.at(rxInd).imag()<<"\n";
	}

      myfile.close();
      myfile2.close();
      myfile3.close();
      myfile4.close();
  }

  AntennaArrayBasicModel::BeamformingVector newBfParam;
  double power = 1 / sqrt (totNoArrayElements);

  uint16_t antennaNum [2];
  antennaNum[0] = m_antenna->GetAntennaNumDim1 ();
  antennaNum[1] = m_antenna->GetAntennaNumDim2 ();

  NS_ASSERT_MSG ( totNoArrayElements == antennaNum[0] * antennaNum[1] , "Channel matrix size mismatch in 4D FFT method");
  // compute the antenna weights
  uint16_t best1= bestColumn % antennaNum[0];
  uint16_t best2= bestColumn / antennaNum[0];
  for (uint16_t ind2 = 0; ind2 < antennaNum[1] ; ind2++)
    {
      for (uint16_t ind1 = 0; ind1 < antennaNum[0] ; ind1++)
	{//this is a conj of the FFT vector, i.e. an IFFT
	  double phase = - 2 * M_PI * ( ind1 * best1 / (double ) antennaNum[0] + ind2 * best2 / (double ) antennaNum[1]);
	  newBfParam.first.push_back (exp (std::complex<double> (0, phase)) * power);
	  //	  NS_LOG_DEBUG(""<<exp (std::complex<double> (0, phase)) * power);
	}
    }

  newBfParam.second = bestColumn; // in this model, beam ID is the look up index of the codebook table

  NS_LOG_DEBUG("Created a 4D FFT Beamforming Vector for device tx "<< m_mobility->GetObject<Node> ()->GetId () <<
	       " pointing at device "<< otherDevice->GetNode ()->GetId ()  <<
	       " using 4DFFT indexes "<< bestRow <<" and "<< bestColumn <<
	       " txFFT indices "<< best1 <<" and "<< best2<<
	       " gain "<< bestGain);

  //SAVE THE NEW BEAM HERE
  // we can make modifications in beamID below without changing map key here
  uint32_t beamKey = GetKey(m_mobility->GetObject<Node> ()->GetId (),otherDevice->GetNode ()->GetId ());

  //update the cache with a new values
  // retrieve the position of the two devices
  Vector aPos = m_mobility->GetPosition ();
  Vector bPos = otherDevice->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
  Ptr<CodebookBFVectorCacheEntry> pCacheValue = Create<CodebookBFVectorCacheEntry> ();
  pCacheValue->m_myPos = aPos;
  pCacheValue->m_otherPos = bPos;
  pCacheValue->m_beamId = newBfParam.second;
  pCacheValue->m_antennaWeights =newBfParam.first;
  pCacheValue->m_generatedTime = Simulator::Now ();
  pCacheValue->txBeamInd=bestColumn;
  pCacheValue->rxBeamInd=bestRow;
  pCacheValue->m_equivalentChanCoefs=channelInfo;//chan info is 4DFFT'd, and therefore its matrix coefficients correspond to equivalent channel values
  m_vectorCache[beamKey] = pCacheValue;//


  return( newBfParam );
}

} // namespace mmwave
} // namespace ns3
