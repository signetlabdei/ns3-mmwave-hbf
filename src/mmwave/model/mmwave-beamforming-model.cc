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
#include "ns3/mobility-model.h"
#include "ns3/pointer.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/log.h"

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
MmWaveDftBeamforming::SetBeamformingVectorForDevice (Ptr<NetDevice> otherDevice)
{
  NS_LOG_FUNCTION (this);

  AntennaArrayBasicModel::complexVector_t antennaWeights;

  // retrieve the position of the two devices
  Vector aPos = m_mobility->GetPosition ();

  NS_ASSERT_MSG (otherDevice->GetNode (), "the device " << otherDevice << " is not associated to a node");
  NS_ASSERT_MSG (otherDevice->GetNode ()->GetObject<MobilityModel> (), "the device " << otherDevice << " has not a mobility model");
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

  // compute the antenna weights
  for (uint32_t ind = 0; ind < totNoArrayElements; ind++)
    {
      Vector loc = m_antenna->GetAntennaLocation (ind);
      double phase = -2 * M_PI * (sin (vAngleRadian) * cos (hAngleRadian) * loc.x
                                  + sin (vAngleRadian) * sin (hAngleRadian) * loc.y
                                  + cos (vAngleRadian) * loc.z);
      antennaWeights.push_back (exp (std::complex<double> (0, phase)) * power);
    }

  // configure the antenna to use the new beamforming vector
  AntennaArrayBasicModel::BeamId bId = 0; // TODO how to set the bid?
  m_antenna->SetBeamformingVector (antennaWeights, bId, otherDevice);
}

} // namespace mmwave
} // namespace ns3
