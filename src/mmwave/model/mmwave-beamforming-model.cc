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
#include "ns3/three-gpp-propagation-loss-model.h"
#include "mmwave-spectrum-value-helper.h"

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
MmWaveBeamformingModel::SetMobilityModel (Ptr<MobilityModel> mm)
{
  NS_LOG_FUNCTION (this);

  m_mobility = mm;
}


void
MmWaveBeamformingModel::SetPropagationLossModel (Ptr<PropagationLossModel> propagationLossModel)
{
  NS_LOG_FUNCTION (this);
  m_propagationLossModel = propagationLossModel;
}

void
MmWaveBeamformingModel::SetSpectrumPropagationLossModel (Ptr<SpectrumPropagationLossModel> spectrumPropagationLossModel)
{
  NS_LOG_FUNCTION (this);
  m_spectrumPropagationLossModel = spectrumPropagationLossModel;
}


void
MmWaveBeamformingModel::SetConfigurationParameters (Ptr<MmWavePhyMacCommon> ptrConfig)
{
  NS_LOG_FUNCTION (this);
  m_mmWavePhyMacConfig = ptrConfig;
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
      castAntenna->ToggleDigitalCombining( false );
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
MmWaveFFTCodebookBeamforming::InPlaceArrayFFT (ComplexArray_t& x, bool inv)
{
  //This method is a direct transposition of the fft examples provided in https://rosettacode.org/wiki/Fast_Fourier_transform#C.2B.2B
  //The code in this method implementation is under the GNU Free Documentation License 1.2  https://www.gnu.org/licenses/old-licenses/fdl-1.2.html
  //------------------------------------------------------------------------
  const  uint16_t N = x.size();
  if (N <= 1) return;

  if ( inv )
    {//the IFFT uses the FFT internally, only conjugate at the beginning and end
      x=x.apply(std::conj);
    }

  // divide
  ComplexArray_t even = x[std::slice(0, N/2, 2)];
  ComplexArray_t  odd = x[std::slice(1, N/2, 2)];

  // conquer
  InPlaceArrayFFT(even,false);
  InPlaceArrayFFT(odd,false);

  // combine
  for ( uint16_t k = 0; k < N/2; ++k)
    {
      std::complex<double> t = std::polar(1.0, -2 * PI * k / N) * odd[k];
      x[k    ] = even[k] + t;
      x[k+N/2] = even[k] - t;
    }
  //--------------------------------------------------------------------------

  if ( inv )
    {
      x=x.apply(std::conj);
    }
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
	  InPlaceArrayFFT(x,false); //in-place FFT of the segment
	  for ( uint16_t colItem = 0; colItem < antennaNum[0]; colItem++)//TODO can we replace this for with native stl subvector methods?
	    {
	      matrix.at(row).at ( colItem +  antennaNum[0]*colSegment ) = x[colItem] / sqrt( (double )antennaNum[0] ); // replace the segment with its energy-normalized FFT
	    }
	  rowItemIterator1 = rowItemIterator2;
	  rowItemIterator2 += antennaNum[0];//move iterators to next segment
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
	    InPlaceArrayFFT(x,false); //in-place FFT of the segment
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
	    InPlaceArrayFFT(x,false); //in-place FFT of the segment
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
	    InPlaceArrayFFT(x,false); //in-place FFT of the segment
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
  Ptr<ThreeGppSpectrumPropagationLossModel> casted3GPPchan = DynamicCast<ThreeGppSpectrumPropagationLossModel>( m_spectrumPropagationLossModel );

  if ( casted3GPPchan !=0 )
    {
      //TODO use the channel update time instead of this independent update timer
      return( ( Simulator::Now ().GetNanoSeconds () - pCacheCasted->m_generatedTime.GetNanoSeconds () ) > 100000 );
    }
  else
    {
      //fallback behavior, if we are not using the 3GPP channel and cannot reat its update time, we only reuse bf caches generated on the same time instant
      return( ( Simulator::Now ().GetNanoSeconds () - pCacheCasted->m_generatedTime.GetNanoSeconds () ) > 0 );
    }
}
std::pair<uint16_t,uint16_t>
MmWaveFFTCodebookBeamforming::bfGainLookup(complex2DVector_t& equivalentChannelCoefs, std::set<uint16_t> blockedTxIdx)
{
  uint16_t bestColumn = 0;
  uint16_t bestRow = 0;
  double bestGain = 0;

  //  std::stringstream matrixline;
  //  matrixline << "[";
  // NS_LOG_DEBUG("4DFFT of channel Matrix: ");
   for ( uint16_t rxInd=0; rxInd<equivalentChannelCoefs.size(); rxInd++)
     {
       for ( uint16_t txInd=0; txInd<equivalentChannelCoefs.at(0).size(); txInd++)
         {
           double testGain = norm( equivalentChannelCoefs.at(rxInd).at(txInd) ) ;
  //        matrixline << (txInd == 0 ? "" : ",") << std::real(channelInfo.at(rxInd).at(txInd))<< "+1i*"<<std::imag(channelInfo.at(rxInd).at(txInd));

           if ( ( blockedTxIdx.find( txInd ) == blockedTxIdx.end() ) & ( testGain > bestGain ) )
             {
               bestColumn = txInd;
               bestRow    = rxInd;//this is not used by me, this is the BF vector I assume the other Device will use if they apply the same bf as me, with transposed matrix
               bestGain   = testGain;
             }
         }
  //      if ( rxInd < ( channelInfo.size() - 1 ) )
  //        {
  //          matrixline << ";";
  //        }
  //      NS_LOG_DEBUG(matrixline.str());
  //      matrixline.str("");
     }
  //  NS_LOG_DEBUG("]");
   return std::pair<uint16_t,uint16_t>(bestRow,bestColumn);
}

complexVector_t
MmWaveFFTCodebookBeamforming::bfVector2DFFT(uint16_t index, uint16_t antennaNum [2])
{
  complexVector_t newVector;
// compute the antenna weights
  uint16_t best1= index % antennaNum [0];
  uint16_t best2= index / antennaNum [0];
  double power = 1 / sqrt ( antennaNum[1] * antennaNum[0]);
  for (uint16_t ind2 = 0; ind2 < antennaNum[1] ; ind2++)
    {
      for (uint16_t ind1 = 0; ind1 < antennaNum[0] ; ind1++)
        {//this is a conj of the FFT vector, i.e. an IFFT
          double phase = - 2 * M_PI * ( ind1 * best1 / (double ) antennaNum[0] + ind2 * best2 / (double ) antennaNum[1]);
          newVector.push_back (exp (std::complex<double> (0, phase)) * power);
          //      NS_LOG_DEBUG(""<<exp (std::complex<double> (0, phase)) * power);
        }
    }
  return newVector;
}

AntennaArrayBasicModel::BeamformingVector
MmWaveFFTCodebookBeamforming::DoDesignBeamformingVectorForDevice (Ptr<NetDevice> otherDevice)
{

  Ptr<ThreeGppSpectrumPropagationLossModel> casted3GPPchan = DynamicCast<ThreeGppSpectrumPropagationLossModel>( m_spectrumPropagationLossModel );

  NS_ASSERT_MSG ( casted3GPPchan != 0, "The spectrum propagation loss model in the channel does not support this BF model");
  //TODO it is theoretically possible to build a 2D channel info using angular sampling with a series of calls to the antenna array radiation pattern, but we will not implement this at this time

  //TODO put here the deltaFc corresponding to the subcarrier number of the narrowband reference signal in NR
  double deltaf = 0; //MmWaveSpectrumValueHelper::GetSpectrumModel ()-> Begin ()-> fc - casted3GPPchan->GetFrequency();
  complex2DVector_t channelInfo = casted3GPPchan->GetFrequencyFlatChannelMatrixAtDeltaFrequency(m_mobility,otherDevice->GetNode ()->GetObject<MobilityModel> (),deltaf);

  Channel4DFFT( channelInfo,otherDevice);//in place 4 FFTs for all four dimensions of tx and rx array
  //combined, the four FFTs above transoform channelInfo axes from [rxArrayElem,txArrayElem] into [rxRefAngle,txRefAngle]
  //in an ULA the refAngles correspond to static beams, with angular values asin( (0:Nant-1 /Nant) - Nant/2 )
  std::pair<uint16_t,uint16_t> bfPairSelection = bfGainLookup(channelInfo);
  uint16_t bestColumn = bfPairSelection.second;
  uint16_t bestRow = bfPairSelection.first;
  AntennaArrayBasicModel::BeamformingVector newBfParam;

  uint16_t antennaNum [2];
  antennaNum[0] = m_antenna->GetAntennaNumDim1 ();
  antennaNum[1] = m_antenna->GetAntennaNumDim2 ();
  uint32_t totNoArrayElements = antennaNum[0]*antennaNum[1];

  NS_ASSERT_MSG ( channelInfo.at(0).size() == totNoArrayElements , "Channel matrix size mismatch in 4D FFT method");

  newBfParam.first = bfVector2DFFT ( bestColumn, antennaNum );
  newBfParam.second = bestColumn; // in this model, beam ID is the look up index of the codebook table

  NS_LOG_DEBUG("Created a 4D FFT Beamforming Vector for device tx "<< m_mobility->GetObject<Node> ()->GetId () <<
	       " pointing at device "<< otherDevice->GetNode ()->GetId ()  <<
	       " using 4DFFT indexes "<< bestRow <<" and "<< bestColumn << " gain "<< std::norm(channelInfo.at(bestRow).at(bestColumn)));

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



/*----------------------------------------------------------------------------*/

NS_OBJECT_ENSURE_REGISTERED (MmWaveMMSEBeamforming);

TypeId
MmWaveMMSEBeamforming::GetTypeId ()
{
  static TypeId
    tid =
    TypeId ("ns3::MmWaveMMSEBeamforming")
    .SetParent<MmWaveFFTCodebookBeamforming> ()
    .AddConstructor<MmWaveMMSEBeamforming> ()
  ;
  return tid;
}

MmWaveMMSEBeamforming::MmWaveMMSEBeamforming ()
{
  NS_LOG_FUNCTION (this);
}

MmWaveMMSEBeamforming::~MmWaveMMSEBeamforming ()
{

}

void
MmWaveMMSEBeamforming::SetNoisePowerSpectralDensity( double noisePSD )
{
  m_noisePowerSpectralDensity = noisePSD;
}

complex2DVector_t
MmWaveMMSEBeamforming::MmseCholesky (complex2DVector_t matrixH)
{
  //This method obtains the cholesky decomposition of the positive definite hermitian matrix M=(H'H+NoI),
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
	  std::complex<double> sum = ( kcol == kdiag )? m_noisePowerSpectralDensity : 0;
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
MmWaveMMSEBeamforming::MmseSolve (complex2DVector_t matrixH, complexVector_t v)
{
  //Cholesky linear solver for x of linear system (H'*H+No*I)x=H'v, returning x=(H'*H+No*I)^-1H'v
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

std::vector< Ptr<CodebookBFVectorCacheEntry>>
MmWaveMMSEBeamforming::GetBfCachesInSlotBundle(std::vector< Ptr<NetDevice> > vOtherDevs)
{
  std::vector< Ptr<CodebookBFVectorCacheEntry>> bfCachesInSlot;
  std::set< uint16_t > txBeamsCollection;
  for (std::vector< Ptr<NetDevice> >::iterator itDev = vOtherDevs.begin() ; itDev != vOtherDevs.end() ; itDev++ )
    {//retrieve the analog BF cache items, while making sure that the analog BF part is up to date
      bool update = false;
      bool notFound = false;
      uint32_t beamKey = GetKey(m_mobility->GetObject<Node> ()->GetId (), (*itDev )->GetNode ()->GetId ());
      std::map< uint32_t, Ptr<BFVectorCacheEntry> >::iterator itVectorCache = m_vectorCache.find(beamKey);
      Ptr<CodebookBFVectorCacheEntry> bCacheEntry;
      if ( itVectorCache != m_vectorCache.end() )
        {
          NS_LOG_DEBUG ("MMSE retrieved a beam from the map");
          bCacheEntry = DynamicCast<CodebookBFVectorCacheEntry>(itVectorCache->second);
          update = CheckBfCacheExpiration( (*itDev ),  bCacheEntry);
        }
      else
        {
          notFound = true;
        }
      if ( notFound | update ){
          NS_LOG_DEBUG ("MMSE could not retreive beam from map or it has expired, generating new analog beam");
          DoDesignBeamformingVectorForDevice ( (*itDev ) ); //we do not use the output value directly in this call
          bCacheEntry=DynamicCast<CodebookBFVectorCacheEntry>( m_vectorCache[beamKey] );
      }
      if (txBeamsCollection.find(bCacheEntry->txBeamInd)!=txBeamsCollection.end())
        {//if two users employ the same tx beam we have a matrix rank problem and the SINR suffers a lot, hence we adopt an alternative second-best beam
          //TODO this conflict resolution is greedy, we can design better conflict resolution by going back and trying to substitute ALL repeated beams.
          Ptr<CodebookBFVectorCacheEntry> auxBfRecord = Create<CodebookBFVectorCacheEntry> (); //we must create a temporary single-use copy of the bfCache struct
          auxBfRecord->rxBeamInd = bCacheEntry->rxBeamInd;
          auxBfRecord->m_equivalentChanCoefs = bCacheEntry->m_equivalentChanCoefs;
          complex2DVector_t oneColumnAuxChan;
          oneColumnAuxChan.push_back(auxBfRecord->m_equivalentChanCoefs.at(auxBfRecord->rxBeamInd));//this auxiliary vector reduces the following lookup dimensions in rx side
          std::pair<uint16_t,uint16_t> bfPairSelection = bfGainLookup(oneColumnAuxChan,txBeamsCollection);
          //the first component of this pair is actually garbage (always 1) because we used the oneColumnAuxChan variable in the function call
          uint16_t altBeam = bfPairSelection.second;
          uint16_t antennaNum [2];
          antennaNum[0] = m_antenna->GetAntennaNumDim1 ();
          antennaNum[1] = m_antenna->GetAntennaNumDim2 ();
          auxBfRecord->txBeamInd=altBeam;
          auxBfRecord->m_beamId=altBeam;//the analog beam ID is the txBeamInd
          auxBfRecord->m_antennaWeights=bfVector2DFFT(altBeam,antennaNum);
          bfCachesInSlot.push_back( auxBfRecord );
          txBeamsCollection.insert(auxBfRecord->txBeamInd);
          NS_LOG_DEBUG("MMSE BF beam conflict, node " << m_mobility->GetObject<Node> ()->GetId ()<< " pointing at node " << (*itDev )->GetNode ()->GetId () <<
                       " may not use beamID " << bCacheEntry->txBeamInd << " fall back to beamID " << auxBfRecord->txBeamInd <<
                       " BFgain penalty 1/"<< std::norm(auxBfRecord->m_equivalentChanCoefs.at(auxBfRecord->rxBeamInd).at(bCacheEntry->txBeamInd)) / std::norm(auxBfRecord->m_equivalentChanCoefs.at(auxBfRecord->rxBeamInd).at(auxBfRecord->txBeamInd))
          );
        }
      else
        {
          bfCachesInSlot.push_back( bCacheEntry );
          txBeamsCollection.insert(bCacheEntry->txBeamInd);
        }
    }

  return(bfCachesInSlot);
}
void
MmWaveMMSEBeamforming::SetBeamformingVectorForSlotBundle(std::vector< Ptr<NetDevice> > vOtherDevs , std::vector<uint16_t> vLayerInds)
{
  //this segment builds a list of all Nb analog beams in use in this slot
  std::vector< Ptr<CodebookBFVectorCacheEntry>> bfCachesInSlot = GetBfCachesInSlotBundle(vOtherDevs);

  NS_LOG_LOGIC("Started MMSE slot bundle processing. Detected " << bfCachesInSlot.size() << " simultaneous analog beams");

  //this segment builds and equivalent channel Nb x Nb matrix with coefficients Heq_{i,j} = wa_i^H H_j^h b_j g_j
    // wa_i are my analog beamforming vectors, 1 x Nant, hermitian when we are receiving
    // H_j is the physical array MIMO channel towards device j, Nant x Nant2
    // b_j is the transmit beamforming of device j, size Nant_of_j x 1
    // g_j is the pathloss gain towards device j
  //  the diagonal Heq[ii,ii] is the complex gain for beam ii,
  //  and Heq[ii][jj] is the side-lobe cross-interference between beam ii-receiver and beam jj-transmitter
  //  this matrix coefficients are obtained as the conjugates of the matrix stored in the analog beam design, which was implemented in transmission mode
  complex2DVector_t equivalentH;
  double propagationGainAmplitude = 0;
  std::stringstream matrixline;
  matrixline << "[";
  for (std::vector< Ptr<CodebookBFVectorCacheEntry>>::iterator itBfThisDev = bfCachesInSlot.begin() ; itBfThisDev != bfCachesInSlot.end() ; itBfThisDev++ )
    {
      complexVector_t rowEquivalentH;
      std::vector< Ptr<NetDevice> >::iterator itOtherDev = vOtherDevs.begin();
      for (std::vector< Ptr<CodebookBFVectorCacheEntry>>::iterator itBfOtherDev = bfCachesInSlot.begin() ; itBfOtherDev != bfCachesInSlot.end() ; itBfOtherDev++ )
        {
          propagationGainAmplitude = sqrt( pow( 10.0, 0.1 * m_propagationLossModel->CalcRxPower (0, m_mobility, (*itOtherDev)->GetNode ()->GetObject<MobilityModel> ()) ) );
	  rowEquivalentH.push_back( ( propagationGainAmplitude * (*itBfOtherDev)->m_equivalentChanCoefs.at( (*itBfOtherDev)->rxBeamInd ).at( (*itBfThisDev)->txBeamInd ) ) );//w[it2,i]H[it2,i]v[it,i] where i=this node
	  matrixline << (itBfOtherDev == bfCachesInSlot.begin() ? "" : ",") << std::real(rowEquivalentH.back())<< "+1i*"<<std::imag(rowEquivalentH.back());
	  itOtherDev++;
	 }
      matrixline<<";";
      equivalentH.push_back( rowEquivalentH );

    }
  matrixline<<"]";
  NS_LOG_LOGIC("Built the equivalent channel matrix Heq with size " << equivalentH.size() << " x " << equivalentH.at(0).size()<<" : "<<matrixline.str());

  // this segment builds the matrix Wa^H from the antenna array weights in my beamforming vectors. The conjugate is used in reception
  complex2DVector_t analogWtransposed;
  matrixline.str("");
  matrixline << "[";
  uint8_t rowctr = 0 ,colctr = 0 ;
  for ( std::vector< Ptr<CodebookBFVectorCacheEntry> >::iterator itBf = bfCachesInSlot.begin() ; itBf != bfCachesInSlot.end() ; itBf++ )
      {
      colctr = 0 ;
      for ( AntennaArrayBasicModel::complexVector_t::iterator itWcoef = (*itBf)->m_antennaWeights.begin(); itWcoef != (*itBf)->m_antennaWeights.end(); itWcoef++ )
        {
          if ( rowctr == 0 )
            {
              complexVector_t newCol;
              newCol .push_back( (*itWcoef) );
              analogWtransposed .push_back ( newCol );
            }
          else
            {
              matrixline<<(colctr == 0 ? ";" : "");
              analogWtransposed[colctr].push_back( (*itWcoef) );
            }
          matrixline<<(colctr == 0 ? "" : ",") << std::real( (*itWcoef) )<< "+1i*"<<std::imag( (*itWcoef) );
          colctr ++ ;
        }
      rowctr ++ ;
      }
  matrixline<<"]";
  NS_LOG_LOGIC("Built the analog beam matrix Wa^T with size " << analogWtransposed.size() << " x " << analogWtransposed.at(0).size()<<" : "<<matrixline.str());

  //this segment computes a MMSE beamforming-refinement.
  // We want to design W^T = (Heq^HHeq+NoI)^{-1}Heq^H and we want to load Wh^T = W^TWa^T in the array weights
  // Using transposes, we obtain Wh^T by solving the system of equations (Heq^HHeq+I)Wh^T = Heq^HWa^T
  // The variable containing hybridW is returned untransposed
  complex2DVector_t hybridW;
//  double normSq = 0;
  std::vector<double> normSq ( vOtherDevs.size() , 0); //initialization as all-zeros vector of size "size()"
  rowctr=0;
  matrixline.str("");
  matrixline << "[";
  for ( complex2DVector_t::iterator columnIt = analogWtransposed.begin();  columnIt != analogWtransposed.end(); columnIt++ )
    {
      // hybrid beamforming option
      AntennaArrayBasicModel::complexVector_t mmseAntennaWeights =  MmseSolve( equivalentH , (*columnIt) );
      for ( uint8_t i = 0; i < mmseAntennaWeights.size(); i++ )
        {
          if ( rowctr == 0 )
            {
              complexVector_t newCol;
              newCol .push_back( mmseAntennaWeights[i] );
              hybridW .push_back ( newCol );
            }
          else
            {
              hybridW[i].push_back ( mmseAntennaWeights[i] );
            }
          normSq[i] +=  std::norm(mmseAntennaWeights[i]);
        }
      rowctr ++ ;
    }
  rowctr=0;
  for ( complex2DVector_t::iterator rowIt = hybridW.begin();  rowIt != hybridW.end(); rowIt++ )
    {
      for ( complexVector_t::iterator colIt = (*rowIt).begin();  colIt != (*rowIt).end(); colIt++ )
        {//hybrid beamforming must be normalized. Also, we have used transposes instead of conjugates above, so we have computed Wh^H^T, we need to conjugate the coefficients to obtain Wh
          (*colIt) = ( (*colIt) ) / sqrt ( normSq[rowctr] );// this distributes the power of N beams with power allocation
          matrixline<<(colIt == (*rowIt).begin() ? "" : ",") << std::real( (*colIt) )<< "+1i*"<<std::imag( (*colIt) );
        }
      matrixline<<";";
      rowctr ++ ;
    }
  matrixline<<"]";
  NS_LOG_LOGIC("Built the equivalent hybrid beam matrix Wh=WaW with size " << hybridW.size() << " x " << hybridW.at(0).size()<<" : "<<matrixline.str());
  // configure the antenna to use the new beamforming vector
  Ptr<AntennaArrayModel> castAntenna = DynamicCast<AntennaArrayModel>(m_antenna);
  std::vector< Ptr<NetDevice> >::iterator itDev = vOtherDevs.begin();
  std::vector< uint16_t >::iterator itLId = vLayerInds.begin();
  complex2DVector_t::iterator hybridWit = hybridW.begin();
  for (std::vector< Ptr<CodebookBFVectorCacheEntry>>::iterator itBf = bfCachesInSlot.begin() ; itBf != bfCachesInSlot.end() ; itBf++ )
    {
      AntennaArrayBasicModel::BeamId bId = 100+(*itBf)->txBeamInd;//TODO design a beam id value convention for this beamforming technique, possibly based on the cache key value

      NS_LOG_LOGIC("Setting up MMSE antenna weights for UE "<< (*itDev )->GetNode ()->GetId () );

      NS_ASSERT_MSG( castAntenna != 0 , "ERROR: Tried to apply hybrid beamforming to an antenna model without multilayer support");

      castAntenna->SetBeamformingVectorMultilayers ( (*hybridWit), bId, (*itDev), (*itLId));
      castAntenna->ToggleDigitalCombining( false );
      itDev++;
      itLId++;
      hybridWit++;
    }

  //TODO store the MMSE vectors in a separate cache, not the per-user analog beam cache
}


/*----------------------------------------------------------------------------*/


NS_OBJECT_ENSURE_REGISTERED (MmWaveMMSESpectrumBeamforming);

TypeId
MmWaveMMSESpectrumBeamforming::GetTypeId ()
{
  static TypeId
    tid =
    TypeId ("ns3::MmWaveMMSESpectrumBeamforming")
    .SetParent<MmWaveMMSEBeamforming> ()
    .AddConstructor<MmWaveMMSESpectrumBeamforming> ()
  ;
  return tid;
}

MmWaveMMSESpectrumBeamforming::MmWaveMMSESpectrumBeamforming ()
{
  NS_LOG_FUNCTION (this);
}

MmWaveMMSESpectrumBeamforming::~MmWaveMMSESpectrumBeamforming ()
{

}


complex2DVector_t
MmWaveMMSESpectrumBeamforming::MmseSolveSimplified (complex2DVector_t matrixH)
{
  //Cholesky linear solver for x of set of linear systems (H'*H+No*I)X=H', returning X=(H'*H+No*I)^-1H'
  //note: a linear solver is N times faster than a matrix inversion,in fact
  //      the Cholesky matrix inversion algorithm consists in N linear solvers

  complex2DVector_t matrixX;
  complex2DVector_t matrixL=MmseCholesky(matrixH);//define M = H'H+I, factorization M=L*L' with lower-triangular L

  for (uint16_t xRowCtr = 0; xRowCtr < matrixH.at(0).size() ; xRowCtr ++)
    {
      matrixX.push_back(complexVector_t());
    }

  for (uint16_t xColCtr = 0; xColCtr < matrixH.at(0).size() ; xColCtr ++)
    {
      complexVector_t aux1;
      for (uint16_t col = 0; col < matrixH.at(0).size() ; col ++)
         {
           aux1.push_back(std::conj( matrixH.at(xColCtr).at(col) ) );
         }
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

      complexVector_t xCol ( matrixL.size() , 0.0 );// solve L'*x=aux2. We preallocate initialized with zeros because we start writing at the end of the vector
      for (uint16_t revRow = 0; revRow < matrixL.size(); revRow ++ )
        {
          uint16_t row = matrixL.size() -1 - revRow;//we start by the LAST coefficient because L' is upper triangular
          std::complex<double> sum = aux2.at(row);
          for (uint16_t col = row + 1 ; col < matrixL.size() ; col ++)
            {
              sum -= xCol.at(col) * std::conj(matrixL.at(col).at(row));//col-row indexes inverted for Hermitian matrix L'
            }
          xCol[row] = ( sum /  matrixL.at(row).at(row) );
        }
      for (uint16_t xRowCtr = 0; xRowCtr < matrixH.at(0).size() ; xRowCtr ++)
        {
          matrixX.at(xRowCtr).push_back( xCol.at(xRowCtr) );
        }
    }
  return( matrixX );
}


void
MmWaveMMSESpectrumBeamforming::SetBeamformingVectorForSlotBundle(std::vector< Ptr<NetDevice> > vOtherDevs , std::vector<uint16_t> vLayerInds)
{
  Ptr<ThreeGppSpectrumPropagationLossModel> casted3GPPchan = DynamicCast<ThreeGppSpectrumPropagationLossModel>( m_spectrumPropagationLossModel );
  NS_ASSERT_MSG ( casted3GPPchan != 0, "The spectrum propagation loss model in the channel does not support this BF model");
  Ptr<SpectrumModel> spectrumModel = MmWaveSpectrumValueHelper::GetSpectrumModel ( m_mmWavePhyMacConfig );
  Ptr<SpectrumValue> dummyPsd = Create <SpectrumValue> (spectrumModel);

  //this segment builds a list of all Nb analog beams in use in this slot
  std::vector< Ptr<CodebookBFVectorCacheEntry>> bfCachesInSlot = GetBfCachesInSlotBundle( vOtherDevs );

  NS_LOG_DEBUG("Started MMSE slot bundle processing. Detected " << bfCachesInSlot.size() << " simultaneous analog beams");

  //this segment builds and equivalent channel Nb x Nb matrix with coefficients Heq_{i,j} = wa_i^H H_j^h b_j g_j
    // wa_i are my analog beamforming vectors, 1 x Nant, hermitian when we are receiving
    // H_j is the physical array MIMO channel towards device j, Nant x Nant2
    // b_j is the transmit beamforming of device j, size Nant_of_j x 1
    // g_j is the pathloss gain towards device j
  //  the diagonal Heq[ii,ii] is the complex gain for beam ii,
  //  and Heq[ii][jj] is the side-lobe cross-interference between beam ii-receiver and beam jj-transmitter
  //  this matrix coefficients are obtained as the conjugates of the matrix stored in the analog beam design, which was implemented in transmission mode
  std::vector<complex2DVector_t> spectrumEquivH; // [ subband, rxlayer, txlayer], accessing 1 element by subband returns a complex2D vector equivalent MIMO frequency flat channel
  for (size_t sBandCtr = 0 ; sBandCtr < spectrumModel->GetNumBands(); sBandCtr++ ){
      spectrumEquivH.push_back(complex2DVector_t());
      for (uint8_t antCtr = 0 ; antCtr < bfCachesInSlot.size() ; antCtr++ )
        {
          spectrumEquivH.back().push_back(complexVector_t());
        }
  }
  double propagationGainAmplitude = 0;
  std::stringstream matrixline;
  matrixline << "[";
  uint8_t rowCtr = 0;
  for (std::vector< Ptr<CodebookBFVectorCacheEntry>>::iterator itBfThisDev = bfCachesInSlot.begin() ; itBfThisDev != bfCachesInSlot.end() ; itBfThisDev++ )
    {
      std::vector< Ptr<NetDevice> >::iterator itOtherDev = vOtherDevs.begin();
      for (std::vector< Ptr<CodebookBFVectorCacheEntry>>::iterator itBfOtherDev = bfCachesInSlot.begin() ; itBfOtherDev != bfCachesInSlot.end() ; itBfOtherDev++ )
        {
          AntennaArrayBasicModel::BeamformingVector txW ( (*itBfThisDev)->m_antennaWeights , (*itBfThisDev)->txBeamInd );
          uint16_t otherAntennaNum [2];
          otherAntennaNum[0] = sqrt( (*itBfOtherDev)->m_equivalentChanCoefs.size() );
          otherAntennaNum[1] = sqrt( (*itBfOtherDev)->m_equivalentChanCoefs.size() );
          AntennaArrayBasicModel::BeamformingVector rxW ( bfVector2DFFT((*itBfOtherDev)->rxBeamInd,otherAntennaNum) , (*itBfOtherDev)->rxBeamInd );//we did not cache this vector anywhere so we have to retrieve it here
          complexVector_t bfComplexSpectrum = casted3GPPchan->DoCalcRxComplexSpectrum( dummyPsd, m_mobility, (*itOtherDev)->GetNode ()->GetObject<MobilityModel> (), txW, rxW);
          propagationGainAmplitude = sqrt( pow( 10.0, 0.1 * m_propagationLossModel->CalcRxPower (0, m_mobility, (*itOtherDev)->GetNode ()->GetObject<MobilityModel> ()) ) );
          for (size_t sBandCtr = 0 ; sBandCtr < spectrumModel->GetNumBands(); sBandCtr++ ){
              spectrumEquivH.at(sBandCtr).at(rowCtr).push_back( ( propagationGainAmplitude * bfComplexSpectrum.at(sBandCtr) ) );
          }
          matrixline << (itBfOtherDev == bfCachesInSlot.begin() ? "" : ",") << std::real(spectrumEquivH.at(0).at(rowCtr).back())<< "+1i*"<<std::imag(spectrumEquivH.at(0).at(rowCtr).back());
          itOtherDev++;
         }
      rowCtr++;
      matrixline<<";";
    }
  matrixline<<"]";
  NS_LOG_DEBUG("Built the equivalent channel matrix Heq with size " << spectrumEquivH.at(0).size() << " x " << spectrumEquivH.at(0).at(0).size()<<" : "<<matrixline.str());

  std::vector<complex2DVector_t> mmseWDCmatrix;
  for (size_t sBandCtr = 0 ; sBandCtr < spectrumModel->GetNumBands(); sBandCtr++ )
    {
      mmseWDCmatrix.push_back( MmseSolveSimplified( spectrumEquivH.at(sBandCtr) ));
//      matrixline.str("");
//      matrixline << "[";
      for (uint16_t rowCtr=0; rowCtr < mmseWDCmatrix.at(sBandCtr).size(); rowCtr++)
        {//normalize the digital combining so that noise PSD remains No and received power is scaled accordingly for a correct SINR model
        double normSq =0;
        for (uint16_t colCtr=0; colCtr < mmseWDCmatrix.at(sBandCtr).at(rowCtr).size(); colCtr++)
          {
            normSq+=std::norm( mmseWDCmatrix.at(sBandCtr).at(rowCtr).at(colCtr) );
          }
        for (uint16_t colCtr=0; colCtr < mmseWDCmatrix.at(sBandCtr).at(rowCtr).size(); colCtr++)
          {
           mmseWDCmatrix.at(sBandCtr).at(rowCtr).at(colCtr) *= 1.0/std::sqrt( normSq );
//           matrixline<<(colCtr == 0 ? "" : ",") << std::real( mmseWDCmatrix.at(sBandCtr).at(rowCtr).at(colCtr) )<< "+1i*"<<std::imag( mmseWDCmatrix.at(sBandCtr).at(rowCtr).at(colCtr) );
          }
//        matrixline<<(rowCtr == mmseWDCmatrix.at(sBandCtr).size() -1 ? "]" : ";");
        }
//      NS_LOG_DEBUG("Built the digital combining matrix W in subband " << (int) sBandCtr << " with size " << mmseWDCmatrix.at(sBandCtr).size() << " x " << mmseWDCmatrix.at(sBandCtr).at(0).size()<<" : "<<matrixline.str());
    }
  NS_LOG_DEBUG("Built spectrum frequency selective matrices with dimensions "<<mmseWDCmatrix.size()<<" x "<<mmseWDCmatrix.at(0).size()<<" x "<<mmseWDCmatrix.at(0).at(0).size());

  // configure the antenna to use the new beamforming vector
  Ptr<AntennaArrayModel> castAntenna = DynamicCast<AntennaArrayModel>(m_antenna);
  std::vector< Ptr<NetDevice> >::iterator itDev = vOtherDevs.begin();
  std::vector< uint16_t >::iterator itLId = vLayerInds.begin();
  for (std::vector< Ptr<CodebookBFVectorCacheEntry>>::iterator itBf = bfCachesInSlot.begin() ; itBf != bfCachesInSlot.end() ; itBf++ )
    {
      NS_LOG_LOGIC("Setting up MMSE antenna weights for UE "<< (*itDev )->GetNode ()->GetId () );

      NS_ASSERT_MSG( castAntenna != 0 , "ERROR: Tried to apply hybrid beamforming to an antenna model without multilayer support");
      castAntenna->SetBeamformingVectorMultilayers ( (*itBf)->m_antennaWeights, (*itBf)->txBeamInd, (*itDev), (*itLId));
      castAntenna->SetDigitalCombining( mmseWDCmatrix );
      castAntenna->ToggleDigitalCombining( true );
      itDev++;
      itLId++;
    }
}



} // namespace mmwave
} // namespace ns3
