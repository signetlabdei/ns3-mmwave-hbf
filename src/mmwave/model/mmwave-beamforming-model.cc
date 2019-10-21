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

#include <complex>
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

void
MmWaveDftBeamforming::InPlaceMatrixFFT (complex2DVector_t matrix, bool bHorizontal)
{
  // matrix[v][h] is considered the v-th row and h-th column
  uint32_t N = ( bHorizontal == true ) ? (matrix.at(0).size()) : (matrix.size());
  uint32_t numFFTs = ( bHorizontal == true ) ? (matrix.size()) : (matrix.at(0).size());

  if (N <= 1) return;

  // divide
  complex2DVector_t even;
  complex2DVector_t odd;
  if ( bHorizontal == true )
    {
      for (uint32_t row = 0; row < numFFTs; row ++ )
	{
	  even.push_back(complexVector_t(matrix.at(row).begin() , matrix.at(row).end()-N/2));
	  odd.push_back(complexVector_t(matrix.at(row).begin() + N/2 , matrix.at(row).end()));
	}
    }
  else
    {
      even = complex2DVector_t(matrix.begin() , matrix.end()-N/2);
      odd = complex2DVector_t(matrix.begin() + N/2, matrix.end());
    }

  // conquer
  InPlaceMatrixFFT(even,bHorizontal);
  InPlaceMatrixFFT(odd,bHorizontal);

  // combine
  for (size_t k = 0; k < N/2; ++k)
    {
      for (size_t n = 0; n < numFFTs; ++n)
	{
	  if ( bHorizontal == true )
	    {
	      std::complex<double> t = std::polar(1.0, -2 * PI * k / N) * odd[n][k];
	      matrix[n][k    ] = even[n][k] + t;
	      matrix[n][k+N/2] = even[n][k] - t;
	    }
	  else
	    {
	      std::complex<double> t = std::polar(1.0, -2 * PI * k / N) * odd[k][n];
	      matrix[k    ][n] = even[k][n] + t;
	      matrix[k+N/2][n] = even[k][n] - t;
	    }
	}
    }
}

void
MmWaveDftBeamforming::SetBeamformingVectorForDevice (Ptr<NetDevice> otherDevice , uint8_t layerInd)
{
  NS_LOG_FUNCTION (this);

  AntennaArrayBasicModel::complexVector_t antennaWeights;
  AntennaArrayBasicModel::BeamId bId;

  // retrieve the position of the two devices
  Vector aPos = m_mobility->GetPosition ();

  NS_ASSERT_MSG (otherDevice->GetNode (), "the device " << otherDevice << " is not associated to a node");
  NS_ASSERT_MSG (otherDevice->GetNode ()->GetObject<MobilityModel> (), "the device " << otherDevice << " has not a mobility model");
  Vector bPos = otherDevice->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();

  bool update = false;
  bool notFound = false;

  // we can make modifications in beamID below without changing map key here
  uint32_t beamKey = GetKey(m_mobility->GetObject<Node> ()->GetId (),otherDevice->GetNode ()->GetId ());

  std::map< uint32_t, Ptr<BFVectorCacheEntry> >::iterator itVectorCache = m_vectorCache.find(beamKey);
  Ptr<BFVectorCacheEntry> pCacheValue;
  if ( itVectorCache != m_vectorCache.end() )
    {
      NS_LOG_DEBUG ("found a beam in the map");
      pCacheValue = itVectorCache->second; // I should be able to obtain this from itVectorCache without a second map-search, but got lazy
      update = ( aPos.x != pCacheValue->m_myPos.x ) ||
	( aPos.y != pCacheValue->m_myPos.y ) ||
	( aPos.z != pCacheValue->m_myPos.z ) ||
        ( bPos.x != pCacheValue->m_otherPos.x ) ||
        ( bPos.y != pCacheValue->m_otherPos.y ) ||
        ( bPos.z != pCacheValue->m_otherPos.z );
    }
  else
  {
    NS_LOG_DEBUG ("beam NOT found");
    notFound = true;
  }

  if ( notFound || update )
    {
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

    // compute the antenna weights
    for (uint32_t ind = 0; ind < totNoArrayElements; ind++)
      {
	Vector loc = m_antenna->GetAntennaLocation (ind);
	double phase = -2 * M_PI * (sin (vAngleRadian) * cos (hAngleRadian) * loc.x
				    + sin (vAngleRadian) * sin (hAngleRadian) * loc.y
				    + cos (vAngleRadian) * loc.z);
	antennaWeights.push_back (exp (std::complex<double> (0, phase)) * power);
      }

      // bId = 0; // TODO how to set the bid?
      //TODO [fgomez] consider this beamID proposal, the beam is identified by the pair of IDs of the transmitter and receiver
      uint32_t minId = std::min (m_mobility->GetObject<Node> ()->GetId (), otherDevice->GetNode ()->GetId ());
      uint32_t maxId = std::max (m_mobility->GetObject<Node> ()->GetId (), otherDevice->GetNode ()->GetId ());
      bId = GetKey(minId,maxId); //TODO this is the same Cantor function as in ThreeGppChannel::GetKey (minId, maxId), consider unifying

      //update the cache with a new value
      //TODO do we need to garbage collect the old cache value here because it was a pointer?
      pCacheValue = Create<BFVectorCacheEntry> ();
      pCacheValue->m_myPos = aPos;
      pCacheValue->m_otherPos = bPos;
      pCacheValue->m_beamId = bId;
      pCacheValue->m_antennaWeights = antennaWeights;

//      m_vectorCache[beamKey] = pCacheValue;
      itVectorCache->second = pCacheValue; //a faster equivalent to the commented line above
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

} // namespace mmwave
} // namespace ns3
