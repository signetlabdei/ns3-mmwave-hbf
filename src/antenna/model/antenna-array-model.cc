/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
*   Copyright (C) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*   Copyright (c) 2019 SIGNET Lab, Department of Information Engineering,
*   University of Padova
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
*
*/

#include "antenna-array-model.h"
#include <ns3/log.h>
#include <ns3/math.h>
#include <ns3/simulator.h>
#include "ns3/double.h"
#include "ns3/enum.h"
#include <ns3/uinteger.h>
#include <ns3/node.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AntennaArrayModel");

NS_OBJECT_ENSURE_REGISTERED (AntennaArrayModel);

AntennaArrayModel::AntennaArrayModel () : AntennaArrayBasicModel ()
{
  m_omniTx = false;
}

AntennaArrayModel::~AntennaArrayModel ()
{

}

TypeId
AntennaArrayModel::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::AntennaArrayModel")
    .SetParent<AntennaArrayBasicModel> ()
    .AddConstructor<AntennaArrayModel> ()
    .AddAttribute ("AntennaHorizontalSpacing",
                   "Horizontal spacing between antenna elements, in multiples of lambda",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&AntennaArrayModel::m_disH),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("AntennaVerticalSpacing",
                   "Vertical spacing between antenna elements, in multiples of lambda",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&AntennaArrayModel::m_disV),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("AntennaOrientation",
                   "The orentation of the antenna",
                   EnumValue (AntennaOrientation::X0),
                   MakeEnumAccessor(&AntennaArrayModel::SetAntennaOrientation,
                                    &AntennaArrayModel::GetAntennaOrientation),
                   MakeEnumChecker(AntennaOrientation::X0, "X0",
                                   AntennaOrientation::Z0, "Z0",
                                   AntennaOrientation::Y0, "Y0"))
    .AddAttribute ("AntennaNumDim1",
                   "Size of the first dimension of the antenna sector/panel expressed in number of antenna elements",
                    UintegerValue (4),
                    MakeUintegerAccessor (&AntennaArrayModel::SetAntennaNumDim1,&AntennaArrayModel::GetAntennaNumDim1),
                    MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("AntennaNumDim2",
                   "Size of the second dimension of the antenna sector/panel expressed in number of antenna elements",
                    UintegerValue (8),
                    MakeUintegerAccessor (&AntennaArrayModel::SetAntennaNumDim2,&AntennaArrayModel::GetAntennaNumDim2),
                    MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}

double
AntennaArrayModel::GetGainDb (Angles a)
{
  // AntennaArrayModel shall have this gain always set to 0
  // since its antenna gain is already included in GetRadiationPattern calculation
  return 0;
}

void
AntennaArrayModel::SetBeamformingVector (complexVector_t antennaWeights, BeamId beamId,
                                         Ptr<NetDevice> device)
{
  NS_LOG_INFO ("SetBeamformingVector for node id: "<<device->GetNode()->GetId()<<
                 " at:"<<Simulator::Now().GetSeconds());
  m_omniTx = false;
  if (device != nullptr)
    {
      BeamformingStorage::iterator iter = m_beamformingVectorMap.find (device);
      if (iter != m_beamformingVectorMap.end ())
        {
          (*iter).second = std::make_pair (antennaWeights, beamId);
        }
      else
        {
          m_beamformingVectorMap.insert (std::make_pair (device,
                                                         std::make_pair (antennaWeights, beamId)));
        }
      m_beamformingVectorUpdateTimes [device] = Simulator::Now();
    }
  m_currentBeamformingVector = std::make_pair (antennaWeights, beamId);
}

void
AntennaArrayModel::ChangeBeamformingVector (Ptr<NetDevice> device)
{
  m_omniTx = false;
  BeamformingStorage::iterator it = m_beamformingVectorMap.find (device);
  NS_ASSERT_MSG (it != m_beamformingVectorMap.end (), "could not find the beamforming vector for the provided device");
  m_currentBeamformingVector = it->second;
}

AntennaArrayModel::BeamformingVector
AntennaArrayModel::GetCurrentBeamformingVector ()
{
  NS_ABORT_MSG_IF (m_omniTx, "omni transmission do not need beamforming vector");
  return m_currentBeamformingVector;
}

void
AntennaArrayModel::ChangeToOmniTx ()
{
  m_omniTx = true;
}

bool
AntennaArrayModel::IsOmniTx ()
{
  return m_omniTx;
}


AntennaArrayModel::BeamformingVector
AntennaArrayModel::GetBeamformingVector (Ptr<NetDevice> device)
{
  AntennaArrayModel::BeamformingVector beamformingVector;
  BeamformingStorage::iterator it = m_beamformingVectorMap.find (device);
  if (it != m_beamformingVectorMap.end ())
    {
      beamformingVector = it->second;
    }
  else
    {
      beamformingVector = m_currentBeamformingVector;
    }
  return beamformingVector;
}

Time
AntennaArrayModel::GetBeamformingVectorUpdateTime (Ptr<NetDevice> device)
{
  BeamformingStorageUpdateTimes::iterator it = m_beamformingVectorUpdateTimes.find (device);
  NS_ABORT_MSG_IF (it == m_beamformingVectorUpdateTimes.end (), "The beamforming vector for the given device does not exist." );
  return it->second;
}


double
AntennaArrayModel::GetRadiationPattern (double vAngle, double hAngle)
{
  NS_ASSERT_MSG (vAngle >= 0&&vAngle <= 180, "the vertical angle should be the range of [0,180]");
  NS_ASSERT_MSG (hAngle >= -180&&vAngle <= 180, "the vertical angle should be the range of [0,180]");
  return 1;
}

Vector
AntennaArrayModel::GetAntennaLocation (uint8_t index)
{
  Vector loc;

  if (m_orientation == AntennaOrientation::X0)
    {
      //assume the left bottom corner is (0,0,0), and the rectangular antenna array is on the y-z plane.
      loc.x = 0;
      loc.y = m_disH * (index % m_antennaNumDim1);
      loc.z = m_disV * floor (index / m_antennaNumDim1);
    }
  else if (m_orientation == AntennaOrientation::Z0)
    {
      //assume the left bottom corner is (0,0,0), and the rectangular antenna array is on the x-y plane.
      loc.z = 0;
      loc.x = m_disH * (index % m_antennaNumDim1);
      loc.y = m_disV * floor (index / m_antennaNumDim1);
    }
  else
    {
      NS_ABORT_MSG("Not defined antenna orientation");
    }

  return loc;
}

void AntennaArrayModel::SetAntennaOrientation (enum AntennaArrayModel::AntennaOrientation orientation)
{
  m_orientation = orientation;
}

enum AntennaArrayModel::AntennaOrientation
AntennaArrayModel::GetAntennaOrientation () const
{
  return m_orientation;
}

uint8_t
AntennaArrayModel::GetAntennaNumDim1 () const
{
  return m_antennaNumDim1;
}

uint8_t
AntennaArrayModel::GetAntennaNumDim2 () const
{
  return m_antennaNumDim2;
}

void
AntennaArrayModel::SetAntennaNumDim1 (uint8_t antennaNum)
{
  m_antennaNumDim1 = antennaNum;
}

void
AntennaArrayModel::SetAntennaNumDim2 (uint8_t antennaNum)
{
  m_antennaNumDim2 = antennaNum;
}

} /* namespace ns3 */
