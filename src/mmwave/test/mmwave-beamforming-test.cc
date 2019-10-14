/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
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

#include "ns3/mmwave-beamforming-model.h"
#include "ns3/test.h"
#include "ns3/simple-net-device.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/antenna-array-model.h"
#include "ns3/object-factory.h"
#include "ns3/node.h"

NS_LOG_COMPONENT_DEFINE ("MmWaveBeamformingTest");

using namespace ns3;
using namespace mmwave;

/**
* This test case checks if the MmWaveDftBeamforming works properly
*/
class MmWaveBeamformingTestCase : public TestCase
{
public:
  /**
  * Constructor
  */
  MmWaveBeamformingTestCase ();

  /**
  * Destructor
  */
  virtual ~MmWaveBeamformingTestCase ();

private:
  /**
  * Run the test
  */
  virtual void DoRun (void);
};

MmWaveBeamformingTestCase::MmWaveBeamformingTestCase ()
  : TestCase ("Checks if the MmWaveHelper correctly initializes the 3GPP channel model")
{
}

MmWaveBeamformingTestCase::~MmWaveBeamformingTestCase ()
{
}

/**
* Overload the operator << to print the beamforming vectors
*/
std::ostream& operator<<(std::ostream& os, const AntennaArrayModel::complexVector_t& bfVector)
{
  for (auto index : bfVector)
  {
    os << index << " ";
  }
  return os;
}

void
MmWaveBeamformingTestCase::DoRun (void)
{
  // create the mobility model for this device
  Ptr<MobilityModel> mm1 = CreateObject<ConstantPositionMobilityModel> ();
  mm1->SetPosition (Vector (100, 0, 0));

  // create the mobility model for the other device
  Ptr<MobilityModel> mm2 = CreateObject<ConstantPositionMobilityModel> ();
  mm2->SetPosition (Vector (0, 0, 0));

  // create a node for the other device
  Ptr<Node> otherNode = CreateObject<Node> ();

  // aggregate the mobility model to the node
  otherNode->AggregateObject (mm2);

  // create the other net device and associate it with the node
  Ptr<NetDevice> otherDevice = CreateObject<SimpleNetDevice> ();
  otherDevice->SetNode (otherNode);
  otherNode->AddDevice (otherDevice);

  // create the antenna
  Ptr<AntennaArrayBasicModel> antenna = CreateObject<AntennaArrayModel> ();

  // set the number of antenna elements
  antenna->SetAntennaNumDim1 (2);
  antenna->SetAntennaNumDim2 (2);

  // create the beamforming module
  Ptr<MmWaveDftBeamforming> bfModule = CreateObjectWithAttributes<MmWaveDftBeamforming> ("MobilityModel", PointerValue (mm1), "AntennaArray", PointerValue (antenna));

  bfModule->SetBeamformingVectorForDevice (otherDevice);
  AntennaArrayBasicModel::BeamformingVector bfVector = antenna->GetCurrentBeamformingVector ();

  std::cout << AntennaArrayBasicModel::GetVector (bfVector) << std::endl;

  // TODO to be completed
  NS_FATAL_ERROR ("This test needs to be completed");
}

/**
* This suite tests if the beamforming module works properly
*/
class MmWaveBeamformingTest : public TestSuite
{
public:
  MmWaveBeamformingTest ();
};

MmWaveBeamformingTest::MmWaveBeamformingTest ()
  : TestSuite ("mmwave-beamforming-test", UNIT)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new MmWaveBeamformingTestCase, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static MmWaveBeamformingTest mmwaveTestSuite;
